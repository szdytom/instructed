#include "biome.h"
#include "generation.h"

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

void TerrainGenerator::biome_pass(TileMap &tilemap) {
	// Create two RNGs for temperature and humidity noise
	Xoroshiro128PP temp_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	Xoroshiro128PP humidity_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();

	BiomeGenerationPass biome_pass(config_, temp_rng, humidity_rng);
	biome_pass(tilemap);
}

} // namespace istd
