#include "tilemap/pass/smoothen_island.h"
#include "tilemap/biome.h"
#include "tilemap/generation.h"
#include "tilemap/tile.h"
#include <algorithm>
#include <queue>

namespace istd {

SmoothenIslandPass::SmoothenIslandPass(
	const GenerationConfig &config, Xoroshiro128PP rng
)
	: config_(config), noise_(rng) {}

void SmoothenIslandPass::operator()(TileMap &tilemap) {
	remove_small_island(tilemap);
	for (int i = 1; i <= config_.island_smoothen_steps; ++i) {
		smoothen_islands(tilemap, i);
	}
	remove_small_island(tilemap);
}

bool SmoothenIslandPass::is_island_tile(const Tile &tile) const {
	return !(
		tile.base == BaseTileType::Water || tile.base == BaseTileType::Deepwater
		|| tile.base == BaseTileType::Ice
	);
}

void SmoothenIslandPass::remove_small_island(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();
	std::vector<std::vector<bool>> visited(
		map_size * Chunk::size, std::vector<bool>(map_size * Chunk::size, false)
	);

	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			for (std::uint8_t local_x = 0; local_x < Chunk::size; ++local_x) {
				for (std::uint8_t local_y = 0; local_y < Chunk::size;
				     ++local_y) {
					TilePos pos{chunk_x, chunk_y, local_x, local_y};
					auto [global_x, global_y] = pos.to_global();

					// Skip if already visited
					if (visited[global_x][global_y]) {
						continue;
					}

					const Tile &tile = tilemap.get_tile(pos);
					if (!is_island_tile(tile)) {
						visited[global_x][global_y] = true;
						continue;
					}

					// Find connected component of island tiles
					std::vector<TilePos> component_positions;
					std::uint32_t component_size = bfs_component_size(
						tilemap, pos, visited, component_positions
					);

					// Check if this component touches the boundary
					auto boundary_checker =
						[&tilemap](const TilePos &component_pos) {
						return tilemap.is_at_boundary(component_pos);
					};
					bool touches_boundary = std::ranges::any_of(
						component_positions, boundary_checker
					);

					// Skip if it touches the boundary
					if (touches_boundary) {
						continue;
					}

					// If the component is too small, convert it to water
					if (component_size <= config_.island_remove_threshold) {
						for (const auto &island_pos : component_positions) {
							Tile tile = tilemap.get_tile(island_pos);
							tile.base = BaseTileType::Water;
							tilemap.set_tile(island_pos, tile);
						}
					}
				}
			}
		}
	}
}

std::uint32_t SmoothenIslandPass::bfs_component_size(
	TileMap &tilemap, TilePos start_pos,
	std::vector<std::vector<bool>> &visited, std::vector<TilePos> &positions
) {
	std::queue<TilePos> queue;
	queue.push(start_pos);

	auto [start_global_x, start_global_y] = start_pos.to_global();
	visited[start_global_x][start_global_y] = true;

	std::uint32_t size = 0;
	positions.clear();

	while (!queue.empty()) {
		TilePos current = queue.front();
		queue.pop();
		positions.push_back(current);
		++size;

		// Check all neighbors
		std::vector<TilePos> neighbors = tilemap.get_neighbors(current, true);
		for (const auto neighbor : neighbors) {
			auto [neighbor_global_x, neighbor_global_y] = neighbor.to_global();
			if (visited[neighbor_global_x][neighbor_global_y]) {
				continue;
			}

			const Tile &neighbor_tile = tilemap.get_tile(neighbor);
			if (is_island_tile(neighbor_tile)) {
				visited[neighbor_global_x][neighbor_global_y] = true;
				queue.push(neighbor);
			}
		}
	}

	return size;
}

Tile SmoothenIslandPass::ca_tile(
	TilePos pos, Tile tile, const CACtx &ctx
) const {
	constexpr std::uint8_t as_water_chance_map[9] = {
		0, 0, 0, 8, 16, 32, 64, 128, 255
	};

	// Sand -> Water
	auto as_water_chance = as_water_chance_map[ctx.adj_water];
	if (as_water_chance > 0 && tile.base == BaseTileType::Sand) {
		if (ctx.rand < as_water_chance) {
			tile.base = BaseTileType::Water;
			return tile;
		}
	}

	// Water -> Sand
	if (!is_island_tile(tile)) {
		int as_sand_chance = std::clamp(
			ctx.adj_sand * 8 + ctx.adj_land * 32, 0, 255
		);

		if (ctx.rand < as_sand_chance) {
			tile.base = BaseTileType::Sand;
		}
		return tile;
	}

	// Sand -> Land
	if (tile.base == BaseTileType::Sand && ctx.biome == BiomeType::LukeOcean) {
		int as_land_chance = std::clamp(
			256 - ctx.adj_water * 32 - ctx.adj_sand * 12, 0, 255
		);

		if (ctx.rand < as_land_chance) {
			tile.base = BaseTileType::Land;
		}
		return tile;
	}

	// Land -> Sand
	if (tile.base == BaseTileType::Land) {
		int as_sand_chance = std::clamp(
			ctx.adj_water * 32 + ctx.adj_sand * 8, 0, 255
		);

		if (ctx.rand < as_sand_chance) {
			tile.base = BaseTileType::Sand;
		}
	}

	return tile;
}

void SmoothenIslandPass::smoothen_islands_subchunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	SubChunkPos sub_pos, std::uint32_t step_i,
	std::vector<std::pair<TilePos, Tile>> &replacements
) {
	const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);
	auto biome = chunk.get_biome(sub_pos);
	auto biome_props = get_biome_properties(biome);
	if (!biome_props.is_ocean) {
		// Only process ocean biomes
		return;
	}

	auto [start_x, start_y] = subchunk_to_tile_start(sub_pos);
	for (std::uint8_t local_x = start_x;
	     local_x < start_x + Chunk::subchunk_size; ++local_x) {
		for (std::uint8_t local_y = start_y;
		     local_y < start_y + Chunk::subchunk_size; ++local_y) {
			TilePos pos{chunk_x, chunk_y, local_x, local_y};

			Tile tile = tilemap.get_tile(pos);

			auto neighbors = tilemap.get_neighbors(pos, true);
			if (neighbors.size() < 8) {
				continue;
			}

			int adj_land = 0, adj_sand = 0, adj_water = 0;
			for (auto neighbor : neighbors) {
				const Tile &neighbor_tile = tilemap.get_tile(neighbor);
				switch (neighbor_tile.base) {
				case BaseTileType::Land:
					++adj_land;
					break;
				case BaseTileType::Sand:
					++adj_sand;
					break;
				case BaseTileType::Water:
				case BaseTileType::Deepwater:
				case BaseTileType::Ice:
					++adj_water;
					break;
				default:
					break; // Ignore other tile types
				}
			}

			auto [global_x, global_y] = pos.to_global();
			std::uint8_t rand = noise_.noise(global_x, global_y, step_i);

			CACtx ctx{
				biome, rand, adj_land, adj_sand, adj_water,
			};

			Tile new_tile = ca_tile(pos, tile, ctx);
			if (new_tile != tile) {
				replacements.emplace_back(pos, new_tile);
			}
		}
	}

	for (auto [pos, new_tile] : replacements) {
		tilemap.set_tile(pos, new_tile);
	}
}

void SmoothenIslandPass::smoothen_islands(
	TileMap &tilemap, std::uint32_t step_i
) {
	std::vector<std::pair<TilePos, Tile>> replacements;

	for (std::uint8_t chunk_x = 0; chunk_x < tilemap.get_size(); ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < tilemap.get_size();
		     ++chunk_y) {
			for (std::uint8_t sub_x = 0; sub_x < Chunk::subchunk_count;
			     ++sub_x) {
				for (std::uint8_t sub_y = 0; sub_y < Chunk::subchunk_count;
				     ++sub_y) {
					SubChunkPos sub_pos{sub_x, sub_y};
					smoothen_islands_subchunk(
						tilemap, chunk_x, chunk_y, sub_pos, step_i, replacements
					);
				}
			}
		}
	}
}

void TerrainGenerator::smoothen_islands_pass(TileMap &tilemap) {
	SmoothenIslandPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

} // namespace istd
