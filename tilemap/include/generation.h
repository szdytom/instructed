#ifndef TILEMAP_GENERATION_H
#define TILEMAP_GENERATION_H

#include "biome.h"
#include "chunk.h"
#include "noise.h"
#include "tilemap.h"
#include "xoroshiro.h"
#include <cstdint>
#include <vector>

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

class SmoothenMountainsPass {
private:
	const GenerationConfig &config_;
	DiscreteRandomNoise noise_;

	/**
	 * @brief Perform BFS to find connected component size for mountains
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

	/**
	 * @brief Replace mountain tiles with terrain types from neighboring areas
	 * @param tilemap The tilemap to modify
	 * @param positions Vector of mountain positions to replace
	 */
	void demountainize(TileMap &tilemap, const std::vector<TilePos> &positions);

	/**
	 * @brief Remove small mountain components to create smoother terrain
	 * @param tilemap The tilemap to process
	 */
	void remove_small_mountain(TileMap &tilemap);

	/**
	 * @brief Smoothen mountains with cellular automata
	 * @param tilemap The tilemap to process
	 */
	void smoothen_mountains(TileMap &tilemap, std::uint32_t step_i);

	void smoothen_mountains_tile(
		const TileMap &tilemap, TilePos pos, std::uint32_t step_i,
		std::vector<std::pair<TilePos, Tile>> &replacements
	);

public:
	/**
	 * @brief Construct a mountain smoothing pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for terrain replacement
	 */
	SmoothenMountainsPass(const GenerationConfig &config, Xoroshiro128PP rng);

	/**
	 * @brief Smoothen mountains in the terrain
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);
};

class SmoothenIslandPass {
private:
	const GenerationConfig &config_;
	DiscreteRandomNoise noise_;

	/**
	 * @brief Perform BFS to find connected component size for islands
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

	/**
	 * @brief Remove small island components to create smoother terrain
	 * @param tilemap The tilemap to process
	 */
	void remove_small_island(TileMap &tilemap);

	/**
	 * @brief Smoothen islands with cellular automata
	 * @param tilemap The tilemap to process
	 */
	void smoothen_islands(TileMap &tilemap, std::uint32_t step_i);

	void smoothen_islands_subchunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		SubChunkPos sub_pos, std::uint32_t step_i,
		std::vector<std::pair<TilePos, Tile>> &replacements
	);

	struct CACtx {
		BiomeType biome;
		std::uint8_t rand;
		int adj_land, adj_sand, adj_water; // Adjacent tile counts
	};

	Tile ca_tile(TilePos pos, Tile tile, const CACtx &ctx) const;

	/**
	 * @brief Check if a tile is part of an island (Land or Sand)
	 * @param tile The tile to check
	 * @return True if the tile is Land or Sand
	 */
	bool is_island_tile(const Tile &tile) const;

public:
	/**
	 * @brief Construct an island smoothing pass
	 * @param config Generation configuration parameters
	 * @param rng Random number generator for terrain replacement
	 */
	SmoothenIslandPass(const GenerationConfig &config, Xoroshiro128PP rng);

	/**
	 * @brief Smoothen islands in the terrain
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);
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
};

class DeepwaterGenerationPass {
private:
	std::uint32_t deepwater_radius_;

public:
	/**
	 * @brief Construct a deepwater generation pass
	 * @param deepwater_radius Radius to check for water tiles around each water
	 * tile (default: 1)
	 */
	explicit DeepwaterGenerationPass(std::uint32_t deepwater_radius);

	/**
	 * @brief Generate deepwater tiles in ocean biomes
	 * @param tilemap The tilemap to process
	 */
	void operator()(TileMap &tilemap);

private:
	/**
	 * @brief Check if a tile position is within a certain radius and all tiles
	 * are water
	 * @param tilemap The tilemap to check
	 * @param center_pos Center position to check around
	 * @param radius Radius to check within
	 * @return True if all tiles within radius are water
	 */

private:
	/**
	 * @brief Process an ocean sub-chunk to generate deepwater tiles
	 * @param tilemap The tilemap to process
	 * @param chunk_x Chunk X coordinate
	 * @param chunk_y Chunk Y coordinate
	 * @param sub_pos Sub-chunk position within the chunk
	 */
	void process_ocean_subchunk(
		TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
		SubChunkPos sub_pos
	);

	/**
	 * @brief Check if a tile position is within a certain radius and all tiles
	 * are water or deepwater
	 * @param tilemap The tilemap to check
	 * @param center_pos Center position to check around
	 * @param radius Radius to check within
	 * @return True if all tiles within radius are water or deepwater
	 */
	bool is_surrounded_by_water(
		const TileMap &tilemap, TilePos center_pos, std::uint32_t radius
	) const;
};

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

/**
 * @brief Generate a tilemap using the new biome-based system
 * @param tilemap The tilemap to generate into
 * @param config Configuration for generation
 */
void map_generate(TileMap &tilemap, const GenerationConfig &config);

} // namespace istd

#endif
