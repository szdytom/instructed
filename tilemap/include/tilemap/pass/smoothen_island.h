#ifndef ISTD_TILEMAP_PASS_SMOOTHEN_ISLAND_H
#define ISTD_TILEMAP_PASS_SMOOTHEN_ISLAND_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

class SmoothenIslandPass {
private:
	const GenerationConfig &config_;
	DiscreteRandomNoise noise_;

	/**
	 * @brief Perform BFS to find connected component size for islands
	 * @param tilemap The tilemap to search
	 * @param start_pos Starting position for BFS
	 * @param visited 2D array tracking visited tiles
	 * @param positions Output vector of positions in this component
	 * @return Size of the connected component
	 */
	std::uint32_t bfs_component_size(
		TileMap &tilemap, TilePos start_pos,
		std::vector<std::vector<bool>> &visited, std::vector<TilePos> &positions
	);

	/**
	 * @brief Remove small island components to create smoother terrain
	 * @param tilemap The tilemap to process
	 */
	void remove_small_island(TileMap &tilemap);

	/**
	 * @brief Smoothen islands with cellular automata
	 * @param tilemap The tilemap to process
	 */
	void smoothen_islands(TileMap &tilemap, std::uint32_t step_i);

	void smoothen_islands_subchunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		SubChunkPos sub_pos, std::uint32_t step_i,
		std::vector<std::pair<TilePos, Tile>> &replacements
	);

	struct CACtx {
		BiomeType biome;
		std::uint8_t rand;
		int adj_land, adj_sand, adj_water; // Adjacent tile counts
	};

	Tile ca_tile(TilePos pos, Tile tile, const CACtx &ctx) const;

	/**
	 * @brief Check if a tile is part of an island (Land or Sand)
	 * @param tile The tile to check
	 * @return True if the tile is Land or Sand
	 */
	bool is_island_tile(const Tile &tile) const;

public:
	/**
	 * @brief Construct an island smoothing pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for terrain replacement
	 */
	SmoothenIslandPass(const GenerationConfig &config, Xoroshiro128PP rng);

	/**
	 * @brief Smoothen islands in the terrain
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);
};

} // namespace istd

#endif
