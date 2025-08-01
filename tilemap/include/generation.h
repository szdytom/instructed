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

	// Noise parameters
	double temperature_scale = 0.005; // Scale for temperature noise
	int temperature_octaves = 3;      // Number of octaves for temperature noise
	double temperature_persistence = 0.4; // Persistence for temperature noise

	double humidity_scale = 0.005;        // Scale for humidity noise
	int humidity_octaves = 3;          // Number of octaves for humidity noise
	double humidity_persistence = 0.4; // Persistence for humidity noise

	double base_scale = 0.08;          // Scale for base terrain noise
	int base_octaves = 3;          // Number of octaves for base terrain noise
	double base_persistence = 0.5; // Persistence for base terrain noise
};

// Terrain generator class that manages the generation process
class TerrainGenerator {
private:
	GenerationConfig config_;

	// Just some random numbers to mask the seeds
	static constexpr std::uint64_t base_seed_mask = 0x06'a9'cb'b1'b3'96'f3'50;
	static constexpr std::uint64_t temperature_seed_mask
		= 0x79'c8'a7'a1'09'99'd0'e3;
	static constexpr std::uint64_t humidity_seed_mask
		= 0x5e'10'be'e4'd2'6f'34'c2;

	UniformPerlinNoise
		base_noise_; // For base terrain generation (uniform distribution)
	PerlinNoise temperature_noise_; // For temperature
	PerlinNoise humidity_noise_;    // For humidity

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
	 * @param tilemap The tilemap to generate biomes into
	 */
	void generate_biomes(TileMap &tilemap);

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

/**
 * @brief Generate a tilemap using the new biome-based system
 * @param tilemap The tilemap to generate into
 * @param config Configuration for generation
 */
void map_generate(TileMap &tilemap, const GenerationConfig &config);

} // namespace istd

#endif
