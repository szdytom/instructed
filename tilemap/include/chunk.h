#ifndef ISTD_TILEMAP_CHUNK_H
#define ISTD_TILEMAP_CHUNK_H
#include "tile.h"
#include <cstdint>

namespace istd {

// Forward declaration
enum class BiomeType : std::uint8_t;

// Represents the position of a tile in the map, using chunk and local
// coordinates
struct TilePos {
	uint8_t chunk_x;
	uint8_t chunk_y;
	uint8_t local_x;
	uint8_t local_y;
};

struct Chunk {
	// Size of a chunk in tiles (64 x 64)
	static constexpr uint8_t size = 64;

	// Each sub-chunk is 16x16 tiles
	static constexpr uint8_t subchunk_size = 16;

	// Number of sub-chunks in each dimension
	static constexpr uint8_t subchunk_count = size / subchunk_size;

	// 64x64 array of tile types
	Tile tiles[size][size];

	// 4x4 array of biomes for sub-chunks
	BiomeType biome[subchunk_count][subchunk_count];
};

} // namespace istd

#endif