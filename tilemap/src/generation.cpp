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
}

void TerrainGenerator::base_tile_type_pass(TileMap &tilemap) {
	BaseTileTypeGenerationPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator(tilemap);
}

} // namespace istd
