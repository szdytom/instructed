#ifndef TILEMAP_GENERATION_H
#define TILEMAP_GENERATION_H

#include "biome.h"
#include "chunk.h"
#include "noise.h"
#include "tilemap.h"
#include <array>
#include <cstdint>
#include <vector>

namespace istd {

struct GenerationConfig {
	std::uint64_t seed = 0; // Seed for random generation

	// Climate noise parameters
	double temperature_scale = 0.005; // Scale for temperature noise
	double humidity_scale = 0.007;    // Scale for humidity noise

	// Base terrain parameters (used as fallback)
	double scale = 0.02;
	int octaves = 4;
	double persistence = 0.5;

	// Legacy thresholds (for compatibility)
	double water_threshold = 0.3;
	double sand_threshold = 0.4;
	double wood_threshold = 0.7;
	double mountain_threshold = 0.8;
};

// Terrain generator class that manages the generation process
class TerrainGenerator {
private:
	GenerationConfig config_;
	PerlinNoise terrain_noise_;
	PerlinNoise temperature_noise_;
	PerlinNoise humidity_noise_;

	// Biome data for current generation (discarded after completion)
	std::vector<std::vector<std::array<std::array<BiomeType, 4>, 4>>>
		chunk_biomes_;

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
	void generate_map(TileMap &tilemap);

private:
	/**
	 * @brief Generate biome data for all chunks
	 * @param map_size Number of chunks per side
	 */
	void generate_biomes(std::uint8_t map_size);

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
		const SubChunkPos &sub_pos, BiomeType biome
	);

	/**
	 * @brief Get climate values at a global position
	 * @param global_x Global X coordinate
	 * @param global_y Global Y coordinate
	 * @return Pair of (temperature, humidity) in range [0,1]
	 */
	std::pair<double, double> get_climate(
		double global_x, double global_y
	) const;

	/**
	 * @brief Determine tile type based on noise value and biome properties
	 * @param noise_value Terrain noise value [0,1]
	 * @param properties Biome properties to use
	 * @return The appropriate tile type
	 */
	Tile determine_tile_type(
		double noise_value, const BiomeProperties &properties
	) const;
};

/**
 * @brief Generate a tilemap using the new biome-based system
 * @param tilemap The tilemap to generate into
 * @param config Configuration for generation
 */
void map_generate(TileMap &tilemap, const GenerationConfig &config);

} // namespace istd

#endif
