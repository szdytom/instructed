#ifndef TILEMAP_PASS_DEEPWATER_H
#define TILEMAP_PASS_DEEPWATER_H

#include "tilemap/tilemap.h"
#include <cstdint>

namespace istd {

class DeepwaterGenerationPass {
private:
	std::uint32_t deepwater_radius_;

public:
	/**
	 * @brief Construct a deepwater generation pass
	 * @param deepwater_radius Radius to check for water tiles around each water
	 * tile
	 */
	explicit DeepwaterGenerationPass(std::uint32_t deepwater_radius);

	/**
	 * @brief Generate deepwater tiles in ocean biomes
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Check if a tile position is within a certain radius and all tiles
	 * are water
	 * @param tilemap The tilemap to check
	 * @param center_pos Center position to check around
	 * @param radius Radius to check within
	 * @return True if all tiles within radius are water
	 */

private:
	/**
	 * @brief Process an ocean sub-chunk to generate deepwater tiles
	 * @param tilemap The tilemap to process
	 * @param chunk_x Chunk X coordinate
	 * @param chunk_y Chunk Y coordinate
	 * @param sub_pos Sub-chunk position within the chunk
	 */
	void process_ocean_subchunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		SubChunkPos sub_pos
	);

	/**
	 * @brief Check if a tile position is within a certain radius and all tiles
	 * are water or deepwater
	 * @param tilemap The tilemap to check
	 * @param center_pos Center position to check around
	 * @param radius Radius to check within
	 * @return True if all tiles within radius are water or deepwater
	 */
	bool is_surrounded_by_water(
		const TileMap &tilemap, TilePos center_pos, std::uint32_t radius
	) const;
};

} // namespace istd

#endif
