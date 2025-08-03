#ifndef ISTD_TILEMAP_PASS_BASE_TILE_TYPE_H
#define ISTD_TILEMAP_PASS_BASE_TILE_TYPE_H

#include "tilemap/biome.h"
#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

class BaseTileTypeGenerationPass {
private:
	const GenerationConfig &config_;
	UniformPerlinNoise base_noise_;

public:
	/**
	 * @brief Construct a base tile type generation pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for base terrain noise
	 */
	BaseTileTypeGenerationPass(
		const GenerationConfig &config, Xoroshiro128PP rng
	);

	/**
	 * @brief Generate base tile types for the entire tilemap
	 * @param tilemap The tilemap to generate base types into
	 */
	void operator()(TileMap &tilemap);

	/**
	 * @brief Generate terrain for a single chunk
	 * @param tilemap The tilemap to modify
	 * @param chunk_x Chunk X coordinate
	 * @param chunk_y Chunk Y coordinate
	 */
	void generate_chunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y
	);

	/**
	 * @brief Generate terrain for a sub-chunk with specific biome
	 * @param tilemap The tilemap to modify
	 * @param chunk_x Chunk X coordinate
	 * @param chunk_y Chunk Y coordinate
	 * @param sub_pos Sub-chunk position within the chunk
	 * @param biome The biome type for this sub-chunk
	 */
	void generate_subchunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		SubChunkPos sub_pos, BiomeType biome
	);

	/**
	 * @brief Determine base terrain type based on noise value and biome
	 * properties
	 * @param noise_value Base terrain noise value [0,1]
	 * @param properties Biome properties to use
	 * @return The appropriate base tile type
	 */
	BaseTileType determine_base_type(
		double noise_value, const BiomeProperties &properties
	) const;
};

} // namespace istd

#endif