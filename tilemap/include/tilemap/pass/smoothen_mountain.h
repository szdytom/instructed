#ifndef ISTD_TILEMAP_PASS_SMOOTHEN_MOUNTAIN_H
#define ISTD_TILEMAP_PASS_SMOOTHEN_MOUNTAIN_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

class SmoothenMountainsPass {
private:
	const GenerationConfig &config_;
	DiscreteRandomNoise noise_;

	/**
	 * @brief Perform BFS to find connected component size for mountains
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
	 * @brief Replace mountain tiles with terrain types from neighboring areas
	 * @param tilemap The tilemap to modify
	 * @param positions Vector of mountain positions to replace
	 */
	void demountainize(TileMap &tilemap, const std::vector<TilePos> &positions);

	/**
	 * @brief Remove small mountain components to create smoother terrain
	 * @param tilemap The tilemap to process
	 */
	void remove_small_mountain(TileMap &tilemap);

	/**
	 * @brief Smoothen mountains with cellular automata
	 * @param tilemap The tilemap to process
	 */
	void smoothen_mountains(TileMap &tilemap, std::uint32_t step_i);

	void smoothen_mountains_tile(
		const TileMap &tilemap, TilePos pos, std::uint32_t step_i,
		std::vector<std::pair<TilePos, Tile>> &replacements
	);

public:
	/**
	 * @brief Construct a mountain smoothing pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for terrain replacement
	 */
	SmoothenMountainsPass(const GenerationConfig &config, Xoroshiro128PP rng);

	/**
	 * @brief Smoothen mountains in the terrain
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);
};

} // namespace istd

#endif
