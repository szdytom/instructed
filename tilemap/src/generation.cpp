#include "generation.h"
#include "biome.h"
#include <cmath>
#include <random>

namespace istd {

TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config)
	, base_noise_(config.seed)
	, surface_noise_(config.seed + 500) // Different seed for surface features
	, temperature_noise_(config.seed + 1000) // Different seed for temperature
	, humidity_noise_(config.seed + 2000)    // Different seed for humidity
{}

void TerrainGenerator::generate_map(TileMap &tilemap) {
	// First, generate biome data for all chunks
	generate_biomes(tilemap);

	// Then generate terrain for each chunk
	std::uint8_t map_size = tilemap.get_size();
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			generate_chunk(tilemap, chunk_x, chunk_y);
		}
	}
}

void TerrainGenerator::generate_biomes(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();

	// Generate biomes for each sub-chunk
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

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

					// Determine biome and store directly in chunk
					BiomeType biome = determine_biome(temperature, humidity);
					chunk.biome[sub_y][sub_x] = biome;
				}
			}
		}
	}
}

void TerrainGenerator::generate_chunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y
) {
	const Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

	// Generate each sub-chunk with its corresponding biome
	for (std::uint8_t sub_y = 0; sub_y < 4; ++sub_y) {
		for (std::uint8_t sub_x = 0; sub_x < 4; ++sub_x) {
			SubChunkPos sub_pos(sub_x, sub_y);
			BiomeType biome = chunk.biome[sub_y][sub_x];
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

			// Generate base terrain noise value
			double base_noise_value = base_noise_.octave_noise(
				global_x * properties.base_scale,
				global_y * properties.base_scale, properties.base_octaves,
				properties.base_persistence
			);

			// Generate surface feature noise value
			double surface_noise_value = surface_noise_.octave_noise(
				global_x * properties.surface_scale,
				global_y * properties.surface_scale, properties.surface_octaves,
				properties.surface_persistence
			);

			// Determine base terrain type
			BaseTileType base_type
				= determine_base_type(base_noise_value, properties);

			// Determine surface feature type
			SurfaceTileType surface_type = determine_surface_type(
				surface_noise_value, properties, base_type
			);

			// Create tile with base and surface components
			Tile tile;
			tile.base = base_type;
			tile.surface = surface_type;

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

BaseTileType TerrainGenerator::determine_base_type(
	double noise_value, const BiomeProperties &properties
) const {
	if (noise_value < properties.water_threshold) {
		return BaseTileType::Water;
	} else if (noise_value < properties.sand_threshold) {
		return BaseTileType::Sand;
	} else if (noise_value < properties.mountain_threshold) {
		return BaseTileType::Mountain;
	} else if (properties.ice_threshold > 0.0
	           && noise_value < properties.ice_threshold) {
		return BaseTileType::Ice;
	} else {
		return BaseTileType::Land;
	}
}

SurfaceTileType TerrainGenerator::determine_surface_type(
	double noise_value, const BiomeProperties &properties,
	BaseTileType base_type
) const {
	// Don't place surface features on water or ice
	if (base_type == BaseTileType::Water || base_type == BaseTileType::Ice) {
		return SurfaceTileType::Empty;
	}

	// Check for surface features based on thresholds
	if (noise_value < properties.wood_threshold) {
		return SurfaceTileType::Wood;
	} else if (noise_value < properties.snow_threshold) {
		return SurfaceTileType::Snow;
	} else {
		return SurfaceTileType::Empty;
	}
}

// Legacy function for backward compatibility
void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator.generate_map(tilemap);
}

} // namespace istd
