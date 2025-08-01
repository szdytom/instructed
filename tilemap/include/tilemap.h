#ifndef ISTD_TILEMAP_TILEMAP_H
#define ISTD_TILEMAP_TILEMAP_H

#include "chunk.h"
#include <cstdint>
#include <vector>

namespace istd {

class TileMap {
private:
	std::uint8_t size_; // Number of chunks in each dimension (n×n)
	std::vector<std::vector<Chunk>> chunks_; // 2D array of chunks

public:
	/**
	 * @brief Construct a TileMap with n×n chunks
	 * @param size Number of chunks in each dimension (max 100)
	 */
	explicit TileMap(std::uint8_t size);

	/**
	 * @brief Get the size of the tilemap (number of chunks per side)
	 */
	std::uint8_t get_size() const {
		return size_;
	}

	/**
	 * @brief Get a reference to a chunk at the given coordinates
	 * @param chunk_x X coordinate of the chunk
	 * @param chunk_y Y coordinate of the chunk
	 */
	Chunk &get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y);
	const Chunk &get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y) const;

	/**
	 * @brief Get a tile at the given position
	 * @param pos The position of the tile
	 */
	Tile &get_tile(TilePos pos);
	const Tile &get_tile(TilePos pos) const;

	/**
	 * @brief Set a tile at the given position
	 * @param pos The position of the tile
	 * @param tile The tile to set
	 */
	void set_tile(TilePos pos, const Tile &tile);
};

} // namespace istd

#endif