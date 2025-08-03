#ifndef TILEMAP_PASS_MINERAL_CLUSTER_H
#define TILEMAP_PASS_MINERAL_CLUSTER_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

/**
 * @brief Generates mineral clusters (Hematite, Titanomagnetite, Gibbsite) on
 * mountain edges
 */
class MineralClusterGenerationPass {
private:
	const GenerationConfig &config_;
	Xoroshiro128PP rng_;
	DiscreteRandomNoise noise_;

public:
	/**
	 * @brief Construct a mineral cluster generation pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for mineral placement
	 * @param noise_rng Random number generator for noise-based operations
	 */
	MineralClusterGenerationPass(
		const GenerationConfig &config, Xoroshiro128PP rng,
		Xoroshiro128PP noise_rng
	);

	/**
	 * @brief Generate mineral clusters on mountain edges
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Generate mineral centers for a specific mineral type
	 * @param tilemap The tilemap to analyze
	 * @param mineral_type The type of mineral to generate
	 * @param density The generation density (out of 255)
	 * @return Vector of positions where mineral clusters should be placed
	 */
	std::vector<TilePos> generate_mineral_centers(
		const TileMap &tilemap, SurfaceTileType mineral_type,
		std::uint16_t density
	);

	/**
	 * @brief Generate a mineral cluster around a center position
	 * @param tilemap The tilemap to modify
	 * @param center Center position for the mineral cluster
	 * @param mineral_type The type of mineral to place
	 */
	void generate_mineral_cluster(
		TileMap &tilemap, TilePos center, SurfaceTileType mineral_type
	);

	/**
	 * @brief Check if a tile is suitable for mineral placement
	 * @param tilemap The tilemap to check
	 * @param pos Position to check
	 * @return True if minerals can be placed at this position
	 */
	bool is_suitable_for_mineral(const TileMap &tilemap, TilePos pos) const;

	/**
	 * @brief Check if a mountain tile is on the edge (adjacent to non-mountain)
	 * @param tilemap The tilemap to check
	 * @param pos Position to check
	 * @return True if this mountain tile is on the edge
	 */
	bool is_mountain_edge(const TileMap &tilemap, TilePos pos) const;

	/**
	 * @brief Calculate minimum distance between mineral clusters based on map
	 * size
	 * @param density The mineral density setting
	 * @return Minimum distance in tiles
	 */
	std::uint32_t calculate_min_mineral_distance(std::uint16_t density) const;
};

} // namespace istd

#endif // TILEMAP_PASS_MINERAL_CLUSTER_H
