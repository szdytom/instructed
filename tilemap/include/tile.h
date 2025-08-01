#ifndef ISTD_TILEMAP_TILE_H
#define ISTD_TILEMAP_TILE_H

#include <cstdint>
#include <stdexcept> // For std::invalid_argument

namespace istd {

// Array of tile types
constexpr const char *_tiles_types[] = {
	"empty", "mountain", "wood", "sand", "water",
};

constexpr std::size_t _tile_types_n
	= sizeof(_tiles_types) / sizeof(_tiles_types[0]);

struct Tile {
	std::uint8_t type;

	/**
	 * @brief Contruct a Tile with the given type.
	 * Use human readable strings as identifier in code without any runtime
	 * overhead
	 * @param type The tile type as a string
	 */
	static consteval Tile from_name(const char *name) {
		// Find the index of the name in the _tiles_types array at compile time
		for (std::size_t i = 0; i < _tile_types_n; ++i) {
			const char *p = name;
			const char *q = _tiles_types[i];
			// Compare strings character by character
			while (*p && *q && *p == *q) {
				++p;
				++q;
			}
			if (*p == '\0' && *q == '\0') {
				return Tile{static_cast<std::uint8_t>(i)};
			}
		}

		throw std::invalid_argument("Invalid tile type name");
	}
};

static_assert(sizeof(Tile) == 1);

} // namespace istd

#endif
