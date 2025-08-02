#include "generation.h"
#include "biome.h"
#include <cmath>
#include <map>
#include <random>
#include <set>
#include <utility>

namespace istd {

BiomeGenerationPass::BiomeGenerationPass(
	const GenerationConfig &config, Xoroshiro128PP r1, Xoroshiro128PP r2
)
	: config_(config), temperature_noise_(r1), humidity_noise_(r2) {
	temperature_noise_.calibrate(
		config.temperature_scale, config.temperature_octaves,
		config.temperature_persistence
	);
	humidity_noise_.calibrate(
		config.humidity_scale, config.humidity_octaves,
		config.humidity_persistence
	);
}

void BiomeGenerationPass::operator()(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();

	// Generate biomes for each sub-chunk
	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (std::uint8_t sub_x = 0; sub_x < Chunk::subchunk_count;
			     ++sub_x) {
				for (std::uint8_t sub_y = 0; sub_y < Chunk::subchunk_count;
				     ++sub_y) {
					// Calculate global position for this sub-chunk's center
					auto [start_x, start_y]
						= subchunk_to_tile_start(SubChunkPos(sub_x, sub_y));
					double global_x = chunk_x * Chunk::size + start_x
					                  + Chunk::subchunk_size / 2;

					double global_y = chunk_y * Chunk::size + start_y
					                  + Chunk::subchunk_size / 2;

					// Get climate values
					auto [temperature, humidity]
						= get_climate(global_x, global_y);

					// Determine biome and store directly in chunk
					BiomeType biome = determine_biome(temperature, humidity);
					chunk.biome[sub_x][sub_y] = biome;
				}
			}
		}
	}
}

std::pair<double, double> BiomeGenerationPass::get_climate(
	double global_x, double global_y
) const {
	// Generate temperature noise (0-1 range)
	double temperature = temperature_noise_.uniform_noise(
		global_x * config_.temperature_scale,
		global_y * config_.temperature_scale
	);

	// Generate humidity noise (0-1 range)
	double humidity = humidity_noise_.uniform_noise(
		global_x * config_.humidity_scale, global_y * config_.humidity_scale
	);

	return {temperature, humidity};
}

BaseTileTypeGenerationPass::BaseTileTypeGenerationPass(
	const GenerationConfig &config, Xoroshiro128PP rng
)
	: config_(config), base_noise_(rng) {
	base_noise_.calibrate(
		config.base_scale, config.base_octaves, config.base_persistence
	);
}

void BaseTileTypeGenerationPass::operator()(TileMap &tilemap) {
	// Generate base tile types for each chunk
	std::uint8_t map_size = tilemap.get_size();
	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			generate_chunk(tilemap, chunk_x, chunk_y);
		}
	}
}

void BaseTileTypeGenerationPass::generate_chunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y
) {
	const Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

	// Generate each sub-chunk with its corresponding biome
	for (std::uint8_t sub_x = 0; sub_x < Chunk::subchunk_count; ++sub_x) {
		for (std::uint8_t sub_y = 0; sub_y < Chunk::subchunk_count; ++sub_y) {
			SubChunkPos sub_pos(sub_x, sub_y);
			BiomeType biome = chunk.get_biome(sub_pos);
			generate_subchunk(tilemap, chunk_x, chunk_y, sub_pos, biome);
		}
	}
}

void BaseTileTypeGenerationPass::generate_subchunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	SubChunkPos sub_pos, BiomeType biome
) {
	const BiomeProperties &properties = get_biome_properties(biome);

	// Get starting tile coordinates for this sub-chunk
	auto [start_x, start_y] = subchunk_to_tile_start(sub_pos);

	// Generate terrain for each tile in the sub-chunk
	for (std::uint8_t local_x = start_x;
	     local_x < start_x + Chunk::subchunk_size; ++local_x) {
		for (std::uint8_t local_y = start_y;
		     local_y < start_y + Chunk::subchunk_size; ++local_y) {
			// Calculate global coordinates
			double global_x = chunk_x * Chunk::size + local_x;
			double global_y = chunk_y * Chunk::size + local_y;

			// Generate base terrain noise value using uniform distribution
			double base_noise_value
				= base_noise_.uniform_noise(global_x, global_y);

			// Determine base terrain type
			BaseTileType base_type
				= determine_base_type(base_noise_value, properties);

			// Create tile with base and surface components
			Tile tile;
			tile.base = base_type;
			tile.surface = SurfaceTileType::Empty;

			// Set the tile
			TilePos pos{chunk_x, chunk_y, local_x, local_y};
			tilemap.set_tile(pos, tile);
		}
	}
}

BaseTileType BaseTileTypeGenerationPass::determine_base_type(
	double noise_value, const BiomeProperties &properties
) const {
	const std::pair<BaseTileType, double> ratios[] = {
		{BaseTileType::Water,    properties.water_ratio},
		{BaseTileType::Ice,      properties.ice_ratio  },
		{BaseTileType::Sand,     properties.sand_ratio },
		{BaseTileType::Land,     properties.land_ratio },
		{BaseTileType::Mountain, 1.0                   },
	};

	for (const auto &[type, ratio] : ratios) {
		if (noise_value < ratio) {
			return type;
		}
		noise_value -= ratio; // Adjust noise value for next type
	}

	std::unreachable();
}

HoleFillPass::HoleFillPass(const GenerationConfig &config): config_(config) {}

void HoleFillPass::operator()(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t total_tiles = map_size * Chunk::size;

	// Create visited array for the entire map
	std::vector<std::vector<bool>> visited(
		total_tiles, std::vector<bool>(total_tiles, false)
	);

	// Process all tiles in the map
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

					// Only process passable tiles
					if (!is_passable(tile.base)) {
						visited[global_x][global_y] = true;
						continue;
					}

					// Find connected component
					std::vector<TilePos> component_positions;
					std::uint32_t component_size = bfs_component_size(
						tilemap, pos, visited, component_positions
					);

					// Check if this component touches the boundary
					bool touches_boundary = false;
					for (const auto component_pos : component_positions) {
						if (tilemap.is_at_boundary(component_pos)) {
							touches_boundary = true;
							break;
						}
					}

					// Fill small holes that don't touch the boundary
					if (!touches_boundary
					    && component_size <= config_.fill_threshold) {
						for (const TilePos &fill_pos : component_positions) {
							Tile fill_tile = tilemap.get_tile(fill_pos);
							fill_tile.base = BaseTileType::Mountain;
							tilemap.set_tile(fill_pos, fill_tile);
						}
					}
				}
			}
		}
	}
}

bool HoleFillPass::is_passable(BaseTileType type) const {
	return type != BaseTileType::Mountain;
}

std::uint32_t HoleFillPass::bfs_component_size(
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
		std::vector<TilePos> neighbors = tilemap.get_neighbors(current);
		for (const auto neighbor : neighbors) {
			auto [neighbor_global_x, neighbor_global_y] = neighbor.to_global();
			if (visited[neighbor_global_x][neighbor_global_y]) {
				continue;
			}

			const Tile &neighbor_tile = tilemap.get_tile(neighbor);
			if (is_passable(neighbor_tile.base)) {
				visited[neighbor_global_x][neighbor_global_y] = true;
				queue.push(neighbor);
			}
		}
	}

	return size;
}

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

void SmoothenMountainsPass::smoothen_mountains(
	TileMap &tilemap, std::uint32_t step_i
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

	for (std::uint8_t chunk_x = 0; chunk_x < tilemap.get_size(); ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < tilemap.get_size();
		     ++chunk_y) {
			for (std::uint8_t local_x = 0; local_x < Chunk::size; ++local_x) {
				for (std::uint8_t local_y = 0; local_y < Chunk::size;
				     ++local_y) {
					TilePos pos{chunk_x, chunk_y, local_x, local_y};
					auto [global_x, global_y] = pos.to_global();
					auto neighbors = tilemap.get_neighbors(pos);

					// Ignore if adjacent to the boundary
					if (neighbors.size() < 4) {
						continue;
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
					const CAConf &conf
						= cellularAutomataConfigurations[mountain_count];
					int rd = noise_.noise(global_x, global_y, step_i) & 0xF;

					Tile &tile = tilemap.get_tile(pos);
					if (tile.base == BaseTileType::Mountain
					    && conf.remove_chance > rd) {
						demountainize(tilemap, {pos});
					} else if (tile.base != BaseTileType::Mountain
					           && conf.fill_chance > rd) {
						tile.base = BaseTileType::Mountain;
					}
				}
			}
		}
	}
}

TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config), master_rng_(config.seed) {}

void TerrainGenerator::operator()(TileMap &tilemap) {
	biome_pass(tilemap);
	base_tile_type_pass(tilemap);
	smoothen_mountains_pass(tilemap);
	hole_fill_pass(tilemap);
}

void TerrainGenerator::biome_pass(TileMap &tilemap) {
	// Create two RNGs for temperature and humidity noise
	Xoroshiro128PP temp_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	Xoroshiro128PP humidity_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();

	BiomeGenerationPass biome_pass(config_, temp_rng, humidity_rng);
	biome_pass(tilemap);
}

void TerrainGenerator::base_tile_type_pass(TileMap &tilemap) {
	BaseTileTypeGenerationPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

void TerrainGenerator::smoothen_mountains_pass(TileMap &tilemap) {
	SmoothenMountainsPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

void TerrainGenerator::hole_fill_pass(TileMap &tilemap) {
	HoleFillPass pass(config_);
	pass(tilemap);
}

void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator(tilemap);
}

} // namespace istd
