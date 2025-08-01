#include "generation.h"
#include "biome.h"
#include <cmath>
#include <random>

namespace istd {

TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config)
	, terrain_noise_(config.seed)
	, temperature_noise_(config.seed + 1000) // Different seed for temperature
	, humidity_noise_(config.seed + 2000)    // Different seed for humidity
{}

void TerrainGenerator::generate_map(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();

	// First, generate biome data for all chunks
	generate_biomes(map_size);

	// Then generate terrain for each chunk
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			generate_chunk(tilemap, chunk_x, chunk_y);
		}
	}

	// Clear biome data to free memory
	chunk_biomes_.clear();
}

void TerrainGenerator::generate_biomes(std::uint8_t map_size) {
	// Initialize biome data storage
	chunk_biomes_.resize(map_size);
	for (auto &row : chunk_biomes_) {
		row.resize(map_size);
	}

	// Generate biomes for each sub-chunk
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			for (std::uint8_t sub_y = 0; sub_y < 4; ++sub_y) {
				for (std::uint8_t sub_x = 0; sub_x < 4; ++sub_x) {
					// Calculate global position for this sub-chunk's center
					double global_x = static_cast<double>(
						chunk_x * Chunk::size + sub_x * 16 + 8
					);
					double global_y = static_cast<double>(
						chunk_y * Chunk::size + sub_y * 16 + 8
					);

					// Get climate values
					auto [temperature, humidity]
						= get_climate(global_x, global_y);

					// Determine biome
					BiomeType biome = determine_biome(temperature, humidity);
					chunk_biomes_[chunk_y][chunk_x][sub_y][sub_x] = biome;
				}
			}
		}
	}
}

void TerrainGenerator::generate_chunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y
) {
	// Generate each sub-chunk with its corresponding biome
	for (std::uint8_t sub_y = 0; sub_y < 4; ++sub_y) {
		for (std::uint8_t sub_x = 0; sub_x < 4; ++sub_x) {
			SubChunkPos sub_pos(sub_x, sub_y);
			BiomeType biome = chunk_biomes_[chunk_y][chunk_x][sub_y][sub_x];
			generate_subchunk(tilemap, chunk_x, chunk_y, sub_pos, biome);
		}
	}
}

void TerrainGenerator::generate_subchunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	const SubChunkPos &sub_pos, BiomeType biome
) {
	const BiomeProperties &properties = get_biome_properties(biome);

	// Get starting tile coordinates for this sub-chunk
	auto [start_x, start_y] = subchunk_to_tile_start(sub_pos);

	// Generate terrain for each tile in the 16x16 sub-chunk
	for (std::uint8_t local_y = start_y; local_y < start_y + 16; ++local_y) {
		for (std::uint8_t local_x = start_x; local_x < start_x + 16;
		     ++local_x) {
			// Calculate global coordinates
			double global_x
				= static_cast<double>(chunk_x * Chunk::size + local_x);
			double global_y
				= static_cast<double>(chunk_y * Chunk::size + local_y);

			// Generate terrain noise value using biome-specific parameters
			double noise_value = terrain_noise_.octave_noise(
				global_x * properties.scale, global_y * properties.scale,
				properties.octaves, properties.persistence
			);

			// Determine tile type based on noise and biome properties
			Tile tile = determine_tile_type(noise_value, properties);

			// Set the tile
			TilePos pos{chunk_x, chunk_y, local_x, local_y};
			tilemap.set_tile(pos, tile);
		}
	}
}

std::pair<double, double> TerrainGenerator::get_climate(
	double global_x, double global_y
) const {
	// Generate temperature noise (0-1 range)
	double temperature = temperature_noise_.octave_noise(
		global_x * config_.temperature_scale,
		global_y * config_.temperature_scale, 3, 0.5
	);

	// Generate humidity noise (0-1 range)
	double humidity = humidity_noise_.octave_noise(
		global_x * config_.humidity_scale, global_y * config_.humidity_scale, 3,
		0.5
	);

	return {temperature, humidity};
}

Tile TerrainGenerator::determine_tile_type(
	double noise_value, const BiomeProperties &properties
) const {
	if (noise_value < properties.water_threshold) {
		return Tile::from_name("water");
	} else if (noise_value < properties.sand_threshold) {
		return Tile::from_name("sand");
	} else if (noise_value < properties.wood_threshold) {
		return Tile::from_name("wood");
	} else if (noise_value < properties.mountain_threshold) {
		return Tile::from_name("mountain");
	} else {
		return Tile::from_name("empty");
	}
}

// Legacy function for backward compatibility
void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator.generate_map(tilemap);
}

} // namespace istd
