#ifndef ISTD_TILEMAP_CHUNK_H
#define ISTD_TILEMAP_CHUNK_H
#include "tile.h"
#include <compare>
#include <cstdint>
#include <functional>
#include <utility>

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
	 * @brief Calculate squared distance to another TilePos
	 * @param other Other TilePos to compare with
	 * @return Squared distance between the two positions
	 */
	std::uint32_t sqr_distance_to(TilePos other) const;

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

	/**
	 * @brief Three-way comparison operator for TilePos
	 * @param lhs Left-hand side TilePos
	 * @param rhs Right-hand side TilePos
	 * @return Strong ordering comparison result
	 */
	friend std::strong_ordering operator<=>(TilePos lhs, TilePos rhs);

	/**
	 * @brief Equality comparison operator for TilePos
	 * @param lhs Left-hand side TilePos
	 * @param rhs Right-hand side TilePos
	 * @return True if positions are equal
	 */
	friend bool operator==(TilePos lhs, TilePos rhs) = default;
};

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

	/**
	 * @brief Get biome for a specific local tile position
	 * @param local_x Local X coordinate within the chunk
	 * @param local_y Local Y coordinate within the chunk
	 * @return Reference to biome type
	 */
	const BiomeType &get_biome(
		std::uint8_t local_x, std::uint8_t local_y
	) const {
		SubChunkPos sub_pos(local_x / subchunk_size, local_y / subchunk_size);
		return get_biome(sub_pos);
	}

	/**
	 * @brief Get biome for a specific TilePos
	 * @param pos TilePos to get the biome for
	 * @return Reference to biome type
	 */
	const BiomeType &get_biome(TilePos pos) const {
		return get_biome(pos.local_x, pos.local_y);
	}
};

/**
 * @brief Get the starting tile coordinates for a sub-chunk
 * @param pos Sub-chunk position
 * @return Pair of starting tile coordinates (x, y)
 */
std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(SubChunkPos pos);

} // namespace istd

template<>
struct std::hash<istd::TilePos> {
	::std::size_t operator()(istd::TilePos pos) const {
		return (static_cast<::std::size_t>(pos.chunk_x) << 24)
			| (static_cast<::std::size_t>(pos.chunk_y) << 16)
			| (static_cast<::std::size_t>(pos.local_x) << 8)
			| static_cast<::std::size_t>(pos.local_y);
	}
};

#endif