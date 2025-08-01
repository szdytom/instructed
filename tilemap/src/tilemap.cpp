#include "tilemap.h"
#include <stdexcept>

namespace istd {

TileMap::TileMap(std::uint8_t size): size_(size) {
	if (size == 0 || size > 100) {
		throw std::invalid_argument("TileMap size must be between 1 and 100");
	}

	// Initialize the 2D vector of chunks
	chunks_.resize(size);
	for (auto &row : chunks_) {
		row.resize(size);
	}
}

Chunk &TileMap::get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y) {
	if (chunk_x >= size_ || chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	return chunks_[chunk_y][chunk_x];
}

const Chunk &TileMap::get_chunk(
	std::uint8_t chunk_x, std::uint8_t chunk_y
) const {
	if (chunk_x >= size_ || chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	return chunks_[chunk_y][chunk_x];
}

Tile &TileMap::get_tile(const TilePos &pos) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_y][pos.chunk_x].tiles[pos.local_y][pos.local_x];
}

const Tile &TileMap::get_tile(const TilePos &pos) const {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_y][pos.chunk_x].tiles[pos.local_y][pos.local_x];
}

void TileMap::set_tile(const TilePos &pos, const Tile &tile) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	chunks_[pos.chunk_y][pos.chunk_x].tiles[pos.local_y][pos.local_x] = tile;
}

} // namespace istd
