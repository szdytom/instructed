#ifndef ISTD_CORE_UNIT_H
#define ISTD_CORE_UNIT_H

#include "entt/entt.hpp"
#include "istd_util/vec2.h"
#include <vector>

namespace istd {

// A Unit is an entity in the world that can move without aligning to the
// tilemap grid, i.e. have floating-point coordinates.

/**
 * @brief Component to unit identification.
 */
struct UnitIdComponent {
	std::uint8_t room_x, room_y;
	std::uint8_t unit_id;
};

/**
 * @brief Component for unit's position and velocity.
 */
struct KinematicsComponent {
	Vec2 position;
	Vec2 velocity;
};

/**
 * @brief Component for unit's device stack.
 *
 * Contains a list of devices (entities) that the unit has.
 */
struct DeviceStackComponent {
	std::vector<entt::entity> devices;
};

} // namespace istd

#endif
