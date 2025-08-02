#ifndef ISTD_TILEMAP_TILE_H
#define ISTD_TILEMAP_TILE_H

#include <cstdint>
#include <stdexcept> // For std::invalid_argument

namespace istd {

enum class BaseTileType : std::uint8_t {
	Land,
	Mountain,
	Sand,
	Water,
	Ice,
	Deepwater,
	_count
};

enum class SurfaceTileType : std::uint8_t {
	Empty,
	_count
};

constexpr std::uint8_t base_tile_count
	= static_cast<std::uint8_t>(BaseTileType::_count);

constexpr std::uint8_t surface_tile_count
	= static_cast<std::uint8_t>(SurfaceTileType::_count);

static_assert(base_tile_count <= 16, "Base tile don't fit in 4 bits");
static_assert(surface_tile_count <= 16, "Surface tile don't fit in 4 bits");

struct Tile {
	BaseTileType base : 4;
	SurfaceTileType surface : 4;
};

static_assert(sizeof(Tile) == 1);

} // namespace istd

#endif
