#ifndef ISTD_TILEMAP_PASS_MOUNTAIN_HOLE_FILL_H
#define ISTD_TILEMAP_PASS_MOUNTAIN_HOLE_FILL_H

#include "tilemap/generation.h"

namespace istd {

class MountainHoleFillPass {
private:
	const GenerationConfig &config_;

public:
	/**
	 * @brief Construct a hole fill pass
	 * @param config Generation configuration parameters
	 */
	explicit MountainHoleFillPass(const GenerationConfig &config);

	/**
	 * @brief Fill small holes in the terrain using BFS
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Check if a tile type is passable for BFS
	 * @param type The base tile type to check
	 * @return True if the tile is passable (not mountain or at boundary)
	 */
	bool is_passable(BaseTileType type) const;

	/**
	 * @brief Perform BFS to find connected component size
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
};

} // namespace istd

#endif
