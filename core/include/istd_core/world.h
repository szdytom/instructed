#ifndef ISTD_CORE_WORLD_H
#define ISTD_CORE_WORLD_H

#include "istd_core/room.h"
#include "tilemap/generation.h"
#include "tilemap/tilemap.h"
#include <vector>

namespace istd {

struct World {
	std::uint32_t tick;
	TileMap tilemap;
	std::vector<std::vector<Room>> rooms;
	entt::registry registry;

	World(std::uint8_t size);

	void generateTilemap(const GenerationConfig &config);
};

} // namespace istd

#endif
