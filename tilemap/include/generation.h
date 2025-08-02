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
	Seed seed;

	// Noise parameters
	double temperature_scale = 0.05; // Scale for temperature noise
	int temperature_octaves = 3;     // Number of octaves for temperature noise
	double temperature_persistence = 0.4; // Persistence for temperature noise

	double humidity_scale = 0.05;         // Scale for humidity noise
	int humidity_octaves = 3;          // Number of octaves for humidity noise
	double humidity_persistence = 0.4; // Persistence for humidity noise

	double base_scale = 0.08;          // Scale for base terrain noise
	int base_octaves = 3;          // Number of octaves for base terrain noise
	double base_persistence = 0.5; // Persistence for base terrain noise
};

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

// Terrain generator class that manages the generation process
class TerrainGenerator {
private:
	GenerationConfig config_;
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
};

/**
 * @brief Generate a tilemap using the new biome-based system
 * @param tilemap The tilemap to generate into
 * @param config Configuration for generation
 */
void map_generate(TileMap &tilemap, const GenerationConfig &config);

} // namespace istd

#endif
