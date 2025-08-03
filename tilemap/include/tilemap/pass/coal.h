#ifndef TILEMAP_PASS_COAL_H
#define TILEMAP_PASS_COAL_H

#include "tilemap/generation.h"
#include "tilemap/noise.h"

namespace istd {

/**
 * @brief Generates coal deposits on suitable terrain using cellular automata
 */
class CoalGenerationPass {
private:
	const GenerationConfig &config_;
	Xoroshiro128PP rng_;
	DiscreteRandomNoise noise_;

public:
	/**
	 * @brief Construct a coal generation pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for coal placement
	 * @param noise_rng Random number generator for noise-based operations
	 */
	CoalGenerationPass(
		const GenerationConfig &config, Xoroshiro128PP rng,
		Xoroshiro128PP noise_rng
	);

	/**
	 * @brief Generate coal deposits using cellular automata
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Generate initial coal seed points for a chunk
	 * @param tilemap The tilemap to analyze
	 * @param chunk_x Chunk X coordinate
	 * @param chunk_y Chunk Y coordinate
	 * @param seeds Output vector to fill with seed positions
	 */
	void chunk_coal_seeds(
		const TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		std::vector<TilePos> &seeds
	);

	/**
	 * @brief Evolve coal deposits using cellular automata
	 * @param tilemap The tilemap to modify
	 * @param initial_seeds Initial coal seed positions
	 */
	void evolve_coal_deposits(
		TileMap &tilemap, const std::vector<TilePos> &initial_seeds
	);

	/**
	 * @brief Check if a tile is suitable for coal placement
	 * @param tilemap The tilemap to check
	 * @param pos Position to check
	 * @return True if coal can be placed at this position
	 */
	bool is_suitable_for_coal(const TileMap &tilemap, TilePos pos) const;

	/**
	 * @brief Get the coal growth probability for a specific biome
	 * @param biome The biome type
	 * @return Growth probability multiplier (x / 255)
	 */
	std::uint8_t get_biome_coal_growth_probability(BiomeType biome) const;

	/**
	 * @brief Count coal neighbors around a position
	 * @param tilemap The tilemap to check
	 * @param pos Position to check around
	 * @return Number of coal neighbors
	 */
	std::uint8_t count_coal_neighbors(
		const TileMap &tilemap, TilePos pos
	) const;
};

} // namespace istd

#endif // TILEMAP_PASS_COAL_H
