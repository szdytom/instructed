#ifndef ISTD_CORE_WORLD_H
#define ISTD_CORE_WORLD_H

#include "istd_core/room.h"
#include "tilemap/tilemap.h"
#include <vector>

namespace istd {

struct World {
	TileMap tilemap;
	std::vector<std::vector<Room>> rooms;
	entt::registry registry;

	World(std::uint8_t size);
};

} // namespace istd

#endif
