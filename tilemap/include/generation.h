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
};

// Terrain generator class that manages the generation process
class TerrainGenerator {
private:
	GenerationConfig config_;
	PerlinNoise base_noise_;        // For base terrain generation
	PerlinNoise surface_noise_;     // For surface feature generation
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

	/**
	 * @brief Determine surface feature type based on noise value and biome
	 * properties
	 * @param noise_value Surface feature noise value [0,1]
	 * @param properties Biome properties to use
	 * @param base_type The base terrain type (affects surface placement)
	 * @return The appropriate surface tile type
	 */
	SurfaceTileType determine_surface_type(
		double noise_value, const BiomeProperties &properties,
		BaseTileType base_type
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
