#ifndef TILEMAP_PASS_OIL_H
#define TILEMAP_PASS_OIL_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

class OilGenerationPass {
private:
	const GenerationConfig &config_;
	Xoroshiro128PP rng_;
	DiscreteRandomNoise noise_;

public:
	/**
	 * @brief Construct an oil generation pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for oil placement
	 * @param noise_rng Random number generator for noise-based operations
	 */
	OilGenerationPass(
		const GenerationConfig &config, Xoroshiro128PP rng,
		Xoroshiro128PP noise_rng
	);

	/**
	 * @brief Generate oil clusters on the tilemap
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Generate oil center positions using Poisson disk sampling
	 * @param tilemap The tilemap to analyze
	 * @return Vector of positions where oil clusters should be placed
	 */
	std::vector<TilePos> generate_oil_centers(const TileMap &tilemap);

	/**
	 * @brief Generate an oil cluster around a center position
	 * @param tilemap The tilemap to modify
	 * @param center Center position for the oil cluster
	 */
	void generate_oil_cluster(TileMap &tilemap, TilePos center);

	/**
	 * @brief Check if a tile is suitable for oil placement
	 * @param tilemap The tilemap to check
	 * @param pos Position to check
	 * @return True if oil can be placed at this position
	 */
	bool is_suitable_for_oil(const TileMap &tilemap, TilePos pos) const;

	/**
	 * @brief Get biome preference multiplier for oil generation (out of 255)
	 * @param biome The biome type to check
	 * @return Preference value (0-255)
	 */
	std::uint8_t get_biome_oil_preference(BiomeType biome) const;

	/**
	 * @brief Calculate minimum distance between oil fields based on map size
	 * @return Minimum distance in tiles
	 */
	std::uint32_t calculate_min_oil_distance() const;
};

} // namespace istd

#endif
