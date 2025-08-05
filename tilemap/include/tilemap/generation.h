#ifndef TILEMAP_GENERATION_H
#define TILEMAP_GENERATION_H

#include "tilemap/tilemap.h"
#include "tilemap/xoroshiro.h"
#include <cstdint>

namespace istd {

/**
 * @brief Configuration parameters for terrain generation
 */
struct GenerationConfig {
	Seed seed;

	// Noise parameters
	double temperature_scale = 0.05; // Scale for temperature noise
	int temperature_octaves = 3;     // Number of octaves for temperature noise
	double temperature_persistence = 0.4; // Persistence for temperature noise

	double humidity_scale = 0.05;         // Scale for humidity noise
	int humidity_octaves = 3;          // Number of octaves for humidity noise
	double humidity_persistence = 0.4; // Persistence for humidity noise

	double base_scale = 0.08;          // Scale for base terrain noise
	int base_octaves = 3;            // Number of octaves for base terrain noise
	double base_persistence = 0.5;   // Persistence for base terrain noise

	int mountain_smoothen_steps = 2; // Number of steps for mountain smoothing
	                                 // cellular automata
	std::uint32_t mountain_remove_threshold = 10; // Threshold for mountain
	                                              // removal

	int island_smoothen_steps = 8; // Number of steps for island smoothing
	                               // cellular automata
	std::uint32_t island_remove_threshold = 8; // Threshold for island removal

	std::uint32_t fill_threshold = 10;  // Fill holes smaller than this size
	std::uint32_t deepwater_radius = 2; // Radius for deepwater generation

	// Oil generation parameters
	std::uint8_t oil_density = 204; // Average oil fields per 255 chunk (~0.8)
	std::uint8_t oil_cluster_min_size = 1; // Minimum tiles per oil cluster
	std::uint8_t oil_cluster_max_size = 7; // Maximum tiles per oil cluster
	                                       // (should be <= 24)
	std::uint8_t oil_base_probe = 128;     // Biome preference multiplier (out
	                                       // of 255)

	// Mineral cluster generation parameters
	std::uint16_t hematite_density = 450;        // ~1.8 per chunk (n / 255)
	std::uint16_t titanomagnetite_density = 300; // ~1.2 per chunk (n / 255)
	std::uint16_t gibbsite_density = 235;        // ~0.9 per chunk (n / 255)

	std::uint8_t mineral_cluster_min_size = 2;   // Minimum tiles per mineral
	                                             // cluster
	std::uint8_t mineral_cluster_max_size = 5;   // Maximum tiles per mineral
	                                             // cluster
	std::uint8_t mineral_base_prob = 192;        // Base probability for mineral
	                                             // placement

	// Coal generation parameters
	std::uint8_t coal_seeds_per_chunk = 3;   // Number of initial coal seeds per
	                                         // chunk
	std::uint8_t coal_evolution_steps = 6;   // Number of cellular automata
	                                         // evolution steps
	std::uint8_t coal_growth_base_prob = 21; // Base probability for coal
	                                         // growth per neighbor (n / 255)
};

// Terrain generator class that manages the generation process
class TerrainGenerator {
private:
	const GenerationConfig &config_;
	Xoroshiro128PP master_rng_;

public:
	/**
	 * @brief Construct a terrain generator with the given configuration
	 * @param config Generation configuration
	 */
	explicit TerrainGenerator(const GenerationConfig &config);

	/**
	 * @brief Generate terrain for the entire tilemap
	 * @param tilemap The tilemap to generate into
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Generate biome data for all chunks
	 * @param tilemap The tilemap to generate biomes into
	 */
	void biome_pass(TileMap &tilemap);

	/**
	 * @brief Generate base tile types for all chunks
	 * @param tilemap The tilemap to generate base types into
	 */
	void base_tile_type_pass(TileMap &tilemap);

	/**
	 * @brief Smoothen mountains in the terrain
	 * @param tilemap The tilemap to process
	 */
	void smoothen_mountains_pass(TileMap &tilemap);

	/**
	 * @brief Smoothen islands in the terrain
	 * @param tilemap The tilemap to process
	 */
	void smoothen_islands_pass(TileMap &tilemap);

	/**
	 * @brief Fill small holes in the terrain
	 * @param tilemap The tilemap to process
	 */
	void mountain_hole_fill_pass(TileMap &tilemap);

	/**
	 * @brief Generate deepwater tiles in ocean biomes
	 * @param tilemap The tilemap to process
	 */
	void deepwater_pass(TileMap &tilemap);

	/**
	 * @brief Generate oil clusters on suitable terrain
	 * @param tilemap The tilemap to process
	 */
	void oil_pass(TileMap &tilemap);

	/**
	 * @brief Generate mineral clusters on suitable terrain
	 * @param tilemap The tilemap to process
	 */
	void mineral_cluster_pass(TileMap &tilemap);

	/**
	 * @brief Generate coal deposits on suitable terrain
	 * @param tilemap The tilemap to process
	 */
	void coal_pass(TileMap &tilemap);
};
/**
 * @brief Generate a tilemap using the new biome-based system
 * @param tilemap The tilemap to generate into
 * @param config Configuration for generation
 */
void map_generate(TileMap &tilemap, const GenerationConfig &config);

} // namespace istd

#endif
