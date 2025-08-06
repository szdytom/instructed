#ifndef ISTD_CORE_DEVICE_VIHICLE_H
#define ISTD_CORE_DEVICE_VIHICLE_H

#include <entt/entt.hpp>
#include <string_view>

namespace istd {

struct VehiclePrototype {
	std::string_view name;
	std::uint32_t ideal_working_mass;
	float max_speed; // in tile/tick
};

struct VehicleComponent {
	const VehiclePrototype *prototype; // life cycle: static
	float heading; // in radians, 0 is South, counter-clockwise
	float speed;   // percentage of max speed, 0.0 to 1.0
};

} // namespace istd

#endif
