#ifndef ISTD_TILEMAP_CHUNK_H
#define ISTD_TILEMAP_CHUNK_H
#include "tile.h"
#include <cstdint>

namespace istd {
// Represents the position of a tile in the map, using chunk and local
// coordinates
struct TilePos {
	uint8_t chunk_x;
	uint8_t chunk_y;
	uint8_t local_x;
	uint8_t local_y;
};

struct Chunk {
	static constexpr uint8_t size = 64;
	Tile tiles[size][size]; // 64x64 array of tile types
};

} // namespace istd

#endif