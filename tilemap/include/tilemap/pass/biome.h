#ifndef ISTD_TILEMAP_PASS_BIOME_H
#define ISTD_TILEMAP_PASS_BIOME_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

class BiomeGenerationPass {
private:
	const GenerationConfig &config_;

	UniformPerlinNoise temperature_noise_;
	UniformPerlinNoise humidity_noise_;

public:
	/**
	 * @brief Construct a biome generation pass
	 * @param config Generation configuration parameters
	 * @param r1 Random number generator for temperature noise
	 * @param r2 Random number generator for humidity noise
	 */
	BiomeGenerationPass(
		const GenerationConfig &config, Xoroshiro128PP r1, Xoroshiro128PP r2
	);

	/**
	 * @brief Generate biomes for the entire tilemap
	 * @param tilemap The tilemap to generate biomes into
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Get climate values at a global position
	 * @param global_x Global X coordinate
	 * @param global_y Global Y coordinate
	 * @return Pair of (temperature, humidity) in range [0,1]
	 */
	std::pair<double, double> get_climate(
		double global_x, double global_y
	) const;
};

} // namespace istd

#endif
