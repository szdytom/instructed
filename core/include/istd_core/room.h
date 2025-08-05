#ifndef ISTD_CORE_ROOM_H
#define ISTD_CORE_ROOM_H

#include "istd_util/small_map.h"
#include "tilemap/chunk.h"
#include <entt/entt.hpp>
#include <tuple>

namespace istd {

// A Room <-> a chunk in tilemap
class Room {
	std::uint8_t chunk_x_, chunk_y_;
	SmallMap<std::tuple<std::uint8_t, std::uint8_t>, entt::entity> structures_;

public:
	Room(std::uint8_t chunk_x, std::uint8_t chunk_y);

	TilePos tilepos_of(std::uint8_t local_x, std::uint8_t local_y) const {
		return {chunk_x_, chunk_y_, local_x, local_y};
	}
};

} // namespace istd

#endif