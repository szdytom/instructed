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
	return chunks_[chunk_x][chunk_y];
}

const Chunk &TileMap::get_chunk(
	std::uint8_t chunk_x, std::uint8_t chunk_y
) const {
	if (chunk_x >= size_ || chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	return chunks_[chunk_x][chunk_y];
}

Tile &TileMap::get_tile(const TilePos &pos) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y];
}

const Tile &TileMap::get_tile(const TilePos &pos) const {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y];
}

void TileMap::set_tile(const TilePos &pos, const Tile &tile) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y] = tile;
}

} // namespace istd
