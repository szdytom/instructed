#ifndef ISTD_TILEMAP_CHUNK_H
#define ISTD_TILEMAP_CHUNK_H
#include "tile.h"
#include <compare>
#include <cstdint>

namespace istd {

// Forward declaration
enum class BiomeType : std::uint8_t;

/**
 * @brief Position within a chunk's sub-chunk grid
 */
struct SubChunkPos {
	std::uint8_t sub_x;
	std::uint8_t sub_y;

	constexpr SubChunkPos(std::uint8_t x, std::uint8_t y): sub_x(x), sub_y(y) {}
};

// Represents the position of a tile in the map, using chunk and local
// coordinates
struct TilePos {
	uint8_t chunk_x;
	uint8_t chunk_y;
	uint8_t local_x;
	uint8_t local_y;

	/**
	 * @brief Convert TilePos to global coordinates
	 * @return Pair of global X and Y coordinates
	 */
	std::pair<std::uint16_t, std::uint16_t> to_global() const;

	/**
	 * @brief Construct a TilePos from global coordinates
	 * @param global_x Global X coordinate
	 * @param global_y Global Y coordinate
	 * @return TilePos corresponding to the global coordinates
	 */
	static TilePos from_global(std::uint16_t global_x, std::uint16_t global_y);
};

/**
 * @brief Three-way comparison operator for TilePos
 * @param lhs Left-hand side TilePos
 * @param rhs Right-hand side TilePos
 * @return Strong ordering comparison result
 */
std::strong_ordering operator<=>(const TilePos &lhs, const TilePos &rhs);

struct Chunk {
	// Size of a chunk in tiles (64 x 64)
	static constexpr uint8_t size = 64;

	// Sub-chunk size in tiles
	static constexpr uint8_t subchunk_size = 4;

	// Number of sub-chunks in each dimension
	static constexpr uint8_t subchunk_count = size / subchunk_size;

	// array of tile types
	Tile tiles[size][size];

	// array of biomes for sub-chunks
	BiomeType biome[subchunk_count][subchunk_count];

	/**
	 * @brief Get biome for a specific sub-chunk position
	 * @param pos Sub-chunk position
	 * @return Reference to biome type
	 */
	BiomeType &get_biome(SubChunkPos pos) {
		return biome[pos.sub_x][pos.sub_y];
	}

	/**
	 * @brief Get biome for a specific sub-chunk position (const version)
	 * @param pos Sub-chunk position
	 * @return Const reference to biome type
	 */
	const BiomeType &get_biome(SubChunkPos pos) const {
		return biome[pos.sub_x][pos.sub_y];
	}
};

/**
 * @brief Get the starting tile coordinates for a sub-chunk
 * @param pos Sub-chunk position
 * @return Pair of starting tile coordinates (x, y)
 */
std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(SubChunkPos pos);

} // namespace istd

#endif