#include "biome.h"
#include "generation.h"

namespace istd {

DeepwaterGenerationPass::DeepwaterGenerationPass(std::uint32_t deepwater_radius)
	: deepwater_radius_(deepwater_radius) {}

void DeepwaterGenerationPass::operator()(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();

	// Iterate through all sub-chunks to check biomes efficiently
	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			const Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			// Process each sub-chunk
			for (std::uint8_t sub_x = 0; sub_x < Chunk::subchunk_count;
			     ++sub_x) {
				for (std::uint8_t sub_y = 0; sub_y < Chunk::subchunk_count;
				     ++sub_y) {
					SubChunkPos sub_pos(sub_x, sub_y);
					BiomeType biome = chunk.get_biome(sub_pos);
					const BiomeProperties &properties = get_biome_properties(
						biome
					);

					// Only process ocean biomes
					if (!properties.is_ocean) {
						continue;
					}

					// Process all tiles in this ocean sub-chunk
					process_ocean_subchunk(tilemap, chunk_x, chunk_y, sub_pos);
				}
			}
		}
	}
}

void DeepwaterGenerationPass::process_ocean_subchunk(
	TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	SubChunkPos sub_pos
) {
	// Get starting tile coordinates for this sub-chunk
	auto [start_x, start_y] = subchunk_to_tile_start(sub_pos);

	// Process all tiles in this sub-chunk
	for (std::uint8_t local_x = start_x;
	     local_x < start_x + Chunk::subchunk_size; ++local_x) {
		for (std::uint8_t local_y = start_y;
		     local_y < start_y + Chunk::subchunk_size; ++local_y) {
			TilePos pos{chunk_x, chunk_y, local_x, local_y};

			// Get the tile at this position
			const Tile &tile = tilemap.get_tile(pos);

			// Only process water tiles
			if (tile.base != BaseTileType::Water) {
				continue;
			}

			// Check if this water tile is surrounded by water/deepwater within
			// the specified radius
			if (is_surrounded_by_water(tilemap, pos, deepwater_radius_)) {
				// Replace water with deepwater
				Tile new_tile = tile;
				new_tile.base = BaseTileType::Deepwater;
				tilemap.set_tile(pos, new_tile);
			}
		}
	}
}

bool DeepwaterGenerationPass::is_surrounded_by_water(
	const TileMap &tilemap, TilePos center_pos, std::uint32_t radius
) const {
	auto [center_global_x, center_global_y] = center_pos.to_global();
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t max_global_coord = map_size * Chunk::size;

	// Check all tiles within the radius
	for (std::int32_t dx = -static_cast<std::int32_t>(radius);
	     dx <= static_cast<std::int32_t>(radius); ++dx) {
		for (std::int32_t dy = -static_cast<std::int32_t>(radius);
		     dy <= static_cast<std::int32_t>(radius); ++dy) {
			std::int32_t check_x = static_cast<std::int32_t>(center_global_x)
				+ dx;
			std::int32_t check_y = static_cast<std::int32_t>(center_global_y)
				+ dy;

			// Check bounds
			if (check_x < 0 || check_y < 0
			    || check_x >= static_cast<std::int32_t>(max_global_coord)
			    || check_y >= static_cast<std::int32_t>(max_global_coord)) {
				return false; // Out of bounds, consider as non-water
			}

			// Convert back to TilePos and check if it's water
			TilePos check_pos = TilePos::from_global(
				static_cast<std::uint16_t>(check_x),
				static_cast<std::uint16_t>(check_y)
			);

			const Tile &check_tile = tilemap.get_tile(check_pos);
			if (check_tile.base != BaseTileType::Water
			    && check_tile.base != BaseTileType::Deepwater) {
				return false; // Found non-water tile within radius
			}
		}
	}

	return true; // All tiles within radius are water or deepwater
}

void TerrainGenerator::deepwater_pass(TileMap &tilemap) {
	DeepwaterGenerationPass pass(config_.deepwater_radius);
	pass(tilemap);
}

} // namespace istd
