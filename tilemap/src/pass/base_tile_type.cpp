#include "tilemap/pass/base_tile_type.h"
#include "tilemap/biome.h"
#include "tilemap/chunk.h"
#include "tilemap/generation.h"
#include <utility>

namespace istd {

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
			double base_noise_value = base_noise_.uniform_noise(
				global_x, global_y
			);

			// Determine base terrain type
			BaseTileType base_type = determine_base_type(
				base_noise_value, properties
			);

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

void TerrainGenerator::base_tile_type_pass(TileMap &tilemap) {
	BaseTileTypeGenerationPass pass(config_, master_rng_);
	master_rng_ = master_rng_.jump_96();
	pass(tilemap);
}

} // namespace istd