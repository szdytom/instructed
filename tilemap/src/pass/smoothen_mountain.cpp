#include "generation.h"
#include <map>
#include <queue>
#include <ranges>
#include <set>

namespace istd {

SmoothenMountainsPass::SmoothenMountainsPass(
	const GenerationConfig &config, Xoroshiro128PP rng
)
	: config_(config), noise_(rng) {}

void SmoothenMountainsPass::operator()(TileMap &tilemap) {
	remove_small_mountain(tilemap);
	for (int i = 1; i <= config_.mountain_smoothen_steps; ++i) {
		smoothen_mountains(tilemap, i);
	}
	remove_small_mountain(tilemap);
}

void SmoothenMountainsPass::remove_small_mountain(TileMap &tilemap) {
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
					if (tile.base != BaseTileType::Mountain) {
						visited[global_x][global_y] = true;
						continue;
					}

					// Find connected component of mountains
					std::vector<TilePos> component_positions;
					std::uint32_t component_size = bfs_component_size(
						tilemap, pos, visited, component_positions
					);

					// If the component touches the boundary, skip it
					bool touches_boundary = false;
					for (auto component_pos : component_positions) {
						if (tilemap.is_at_boundary(component_pos)) {
							touches_boundary = true;
							break;
						}
					}

					// Skip if it touches the boundary
					if (touches_boundary) {
						continue;
					}

					// If the component is too small, smooth it out
					if (component_size <= config_.mountain_remove_threshold) {
						demountainize(tilemap, component_positions);
					}
				}
			}
		}
	}
}

void SmoothenMountainsPass::demountainize(
	TileMap &tilemap, const std::vector<TilePos> &pos
) {
	// Step 1: Look around the mountain to see what should replace it
	std::map<BaseTileType, int> type_count;
	std::set<TilePos> unique_positions;
	for (auto p : pos) {
		auto neighbors = tilemap.get_neighbors(p, true);
		unique_positions.insert(neighbors.begin(), neighbors.end());
	}

	for (auto p : unique_positions) {
		const Tile &tile = tilemap.get_tile(p);
		if (tile.base != BaseTileType::Mountain) {
			type_count[tile.base]++;
		}
	}

	int total_count = 0;
	for (const auto &[type, count] : type_count) {
		total_count += count;
	}

	if (total_count == 0) {
		std::unreachable();
	}

	// Step 2: Replace each mountain tile with a random type based on the counts
	for (const auto &p : pos) {
		Tile tile = tilemap.get_tile(p);
		auto [global_x, global_y] = p.to_global();
		auto sample = noise_.noise(global_x, global_y);
		int index = sample % total_count; // Not perfectly uniform, but works
		                                  // for small counts
		for (const auto [type, count] : type_count) {
			if (index < count) {
				tile.base = type;
				break;
			}
			index -= count;
		}
		tilemap.set_tile(p, tile);
	}
}

std::uint32_t SmoothenMountainsPass::bfs_component_size(
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
			if (neighbor_tile.base == BaseTileType::Mountain) {
				visited[neighbor_global_x][neighbor_global_y] = true;
				queue.push(neighbor);
			}
		}
	}

	return size;
}

void SmoothenMountainsPass::smoothen_mountains_tile(
	const TileMap &tilemap, TilePos pos, std::uint32_t step_i,
	std::vector<std::pair<TilePos, Tile>> &replacements
) {
	struct CAConf {
		int neighbor_count;
		int fill_chance = 0;   // n / 16
		int remove_chance = 0; // n / 16
	};

	// Chance to fill or remove a mountain tile repects to the number of
	// neighboring mountains (0 - 4)
	constexpr CAConf cellularAutomataConfigurations[5] = {
		{0, 0,  12},
        {1, 0,  4 },
        {2, 3,  1 },
        {3, 8,  0 },
        {4, 16, 0 }
	};

	auto [global_x, global_y] = pos.to_global();
	auto neighbors = tilemap.get_neighbors(pos);

	// Ignore if adjacent to the boundary
	if (neighbors.size() < 4) {
		return;
	}

	// Count neighboring mountains
	int mountain_count = 0;
	for (const auto &neighbor : neighbors) {
		const Tile &tile = tilemap.get_tile(neighbor);
		if (tile.base == BaseTileType::Mountain) {
			mountain_count += 1;
		}
	}

	// Get the configuration for this count
	const CAConf &conf = cellularAutomataConfigurations[mountain_count];
	auto sample = noise_.noise(global_x, global_y, step_i);
	auto rd = sample & 0xF;
	auto sel = sample >> 4;

	Tile tile = tilemap.get_tile(pos);
	if (tile.base == BaseTileType::Mountain && conf.remove_chance > rd) {
		auto filterer = [&tilemap](const TilePos &p) {
			auto neighbor_tile = tilemap.get_tile(p);
			return neighbor_tile.base != BaseTileType::Mountain;
		};
		auto non_mountain_neighbors = neighbors | std::views::filter(filterer)
			| std::ranges::to<std::vector>();

		if (!non_mountain_neighbors.empty()) {
			auto n = non_mountain_neighbors.size();
			auto replacement = non_mountain_neighbors[sel % n];
			tile.base = tilemap.get_tile(replacement).base;
			replacements.emplace_back(pos, tile);
		}
	} else if (tile.base != BaseTileType::Mountain && conf.fill_chance > rd) {
		tile.base = BaseTileType::Mountain;
		replacements.emplace_back(pos, tile);
	}
}

void SmoothenMountainsPass::smoothen_mountains(
	TileMap &tilemap, std::uint32_t step_i
) {
	std::vector<std::pair<TilePos, Tile>> replacements;

	for (std::uint8_t chunk_x = 0; chunk_x < tilemap.get_size(); ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < tilemap.get_size();
		     ++chunk_y) {
			for (std::uint8_t local_x = 0; local_x < Chunk::size; ++local_x) {
				for (std::uint8_t local_y = 0; local_y < Chunk::size;
				     ++local_y) {
					TilePos pos{chunk_x, chunk_y, local_x, local_y};
					smoothen_mountains_tile(tilemap, pos, step_i, replacements);
				}
			}
		}
	}

	for (const auto &[pos, new_tile] : replacements) {
		tilemap.set_tile(pos, new_tile);
	}
}

void TerrainGenerator::smoothen_mountains_pass(TileMap &tilemap) {
	SmoothenMountainsPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

} // namespace istd