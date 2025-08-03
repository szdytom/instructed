#ifndef ISTD_TILEMAP_TILE_H
#define ISTD_TILEMAP_TILE_H

#include <cstdint>

namespace istd {

enum class BaseTileType : std::uint8_t {
	Mountain = 0x0,
	Land,
	Sand,
	Water,
	Ice,
	Deepwater,
};

enum class SurfaceTileType : std::uint8_t {
	Empty = 0,
	Oil,
	Hematite,
	Titanomagnetite,
	Gibbsite,

	// Player built structures
	// (not used in generation, but can be placed by player)
	Structure = 0xF,
};

struct Tile {
	BaseTileType base : 4;
	SurfaceTileType surface : 4;
	friend bool operator==(Tile lhs, Tile rhs) = default;
};

static_assert(sizeof(Tile) == 1);

} // namespace istd

#endif
