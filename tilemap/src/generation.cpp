#include "generation.h"
#include "biome.h"
#include <cmath>
#include <random>
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
					std::uint32_t global_x = chunk_x * Chunk::size + local_x;
					std::uint32_t global_y = chunk_y * Chunk::size + local_y;

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
					for (const TilePos &component_pos : component_positions) {
						if (is_at_boundary(tilemap, component_pos)) {
							touches_boundary = true;
							break;
						}
					}

					// Fill small holes that don't touch the boundary
					if (!touches_boundary
					    && component_size < config_.fill_threshold) {
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

	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t start_global_x
		= start_pos.chunk_x * Chunk::size + start_pos.local_x;
	std::uint32_t start_global_y
		= start_pos.chunk_y * Chunk::size + start_pos.local_y;
	visited[start_global_x][start_global_y] = true;

	std::uint32_t size = 0;
	positions.clear();

	while (!queue.empty()) {
		TilePos current = queue.front();
		queue.pop();
		positions.push_back(current);
		++size;

		// Check all neighbors
		std::vector<TilePos> neighbors = get_neighbors(tilemap, current);
		for (const TilePos &neighbor : neighbors) {
			std::uint32_t neighbor_global_x
				= neighbor.chunk_x * Chunk::size + neighbor.local_x;
			std::uint32_t neighbor_global_y
				= neighbor.chunk_y * Chunk::size + neighbor.local_y;

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

std::vector<TilePos> HoleFillPass::get_neighbors(
	TileMap &tilemap, TilePos pos
) const {
	std::vector<TilePos> neighbors;
	std::uint8_t map_size = tilemap.get_size();

	// Calculate global coordinates
	std::uint32_t global_x = pos.chunk_x * Chunk::size + pos.local_x;
	std::uint32_t global_y = pos.chunk_y * Chunk::size + pos.local_y;
	std::uint32_t max_global = map_size * Chunk::size;

	// Four cardinal directions
	const int dx[] = {-1, 1, 0, 0};
	const int dy[] = {0, 0, -1, 1};

	for (int i = 0; i < 4; ++i) {
		int new_global_x = static_cast<int>(global_x) + dx[i];
		int new_global_y = static_cast<int>(global_y) + dy[i];

		// Check bounds
		if (new_global_x >= 0 && new_global_x < static_cast<int>(max_global)
		    && new_global_y >= 0
		    && new_global_y < static_cast<int>(max_global)) {
			// Convert back to chunk and local coordinates
			std::uint8_t new_chunk_x = new_global_x / Chunk::size;
			std::uint8_t new_chunk_y = new_global_y / Chunk::size;
			std::uint8_t new_local_x = new_global_x % Chunk::size;
			std::uint8_t new_local_y = new_global_y % Chunk::size;

			neighbors.push_back(
				{new_chunk_x, new_chunk_y, new_local_x, new_local_y}
			);
		}
	}

	return neighbors;
}

bool HoleFillPass::is_at_boundary(TileMap &tilemap, TilePos pos) const {
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t global_x = pos.chunk_x * Chunk::size + pos.local_x;
	std::uint32_t global_y = pos.chunk_y * Chunk::size + pos.local_y;
	std::uint32_t max_global = map_size * Chunk::size - 1;

	return global_x == 0 || global_x == max_global || global_y == 0
	       || global_y == max_global;
}

TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config), master_rng_(config.seed) {}

void TerrainGenerator::biome_pass(TileMap &tilemap) {
	// Create two RNGs for temperature and humidity noise
	Xoroshiro128PP temp_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	Xoroshiro128PP humidity_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();

	BiomeGenerationPass biome_pass(config_, temp_rng, humidity_rng);
	biome_pass(tilemap);
}

void TerrainGenerator::operator()(TileMap &tilemap) {
	// First, generate biome data for all chunks
	biome_pass(tilemap);

	// Then, generate base tile types based on biomes
	base_tile_type_pass(tilemap);

	// Finally, fill small holes in the terrain
	hole_fill_pass(tilemap);
}

void TerrainGenerator::base_tile_type_pass(TileMap &tilemap) {
	BaseTileTypeGenerationPass pass(config_, master_rng_);
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
