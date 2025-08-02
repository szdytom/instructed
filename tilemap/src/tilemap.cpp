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

Tile &TileMap::get_tile(TilePos pos) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y];
}

const Tile &TileMap::get_tile(TilePos pos) const {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	return chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y];
}

void TileMap::set_tile(TilePos pos, const Tile &tile) {
	if (pos.chunk_x >= size_ || pos.chunk_y >= size_) {
		throw std::out_of_range("Chunk coordinates out of bounds");
	}
	if (pos.local_x >= Chunk::size || pos.local_y >= Chunk::size) {
		throw std::out_of_range("Local coordinates out of bounds");
	}
	chunks_[pos.chunk_x][pos.chunk_y].tiles[pos.local_x][pos.local_y] = tile;
}

bool TileMap::is_at_boundary(TilePos pos) const {
	std::uint8_t map_size = get_size();
	std::uint32_t global_x = pos.chunk_x * Chunk::size + pos.local_x;
	std::uint32_t global_y = pos.chunk_y * Chunk::size + pos.local_y;
	std::uint32_t max_global = map_size * Chunk::size - 1;

	return global_x == 0 || global_x == max_global || global_y == 0
	       || global_y == max_global;
}

std::vector<TilePos> TileMap::get_neighbors(TilePos pos, bool chebyshiv) const {
	std::vector<TilePos> neighbors;
	std::uint8_t map_size = get_size();

	auto [global_x, global_y] = pos.to_global();
	int max_global = map_size * Chunk::size - 1;

	// Four cardinal directions
	const int dx[] = {-1, 1, 0, 0, -1, 1, -1, 1};
	const int dy[] = {0, 0, -1, 1, -1, -1, 1, 1};
	for (int i = 0; i < (chebyshiv ? 8 : 4); ++i) {
		int new_global_x = global_x + dx[i];
		int new_global_y = global_y + dy[i];

		// Check bounds
		if (new_global_x >= 0 && new_global_x <= max_global && new_global_y >= 0
		    && new_global_y <= max_global) {
			neighbors.push_back(
				TilePos::from_global(new_global_x, new_global_y)
			);
		}
	}

	return neighbors;
}

} // namespace istd
