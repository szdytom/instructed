#ifndef ISTD_CORE_SYSTEM_H
#define ISTD_CORE_SYSTEM_H

#include "istd_core/world.h"
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

namespace istd {

struct System {
	virtual void tick(World &world) const = 0;
	virtual std::string_view name() const noexcept = 0; // for debugging

	// No virtual destructor: static lifetime and no member variables is the
	// intended use case

	struct Precedence {
		// Smaller value means higher precedence or earlier execution
		enum {
			Highest = 0,
			ResetVelocity,
			DeviceAccumulateVelocity,
			UpdateKinematics,
		};
	};
};

class SystemRegistry {
public:
	static SystemRegistry &instance() noexcept;

	void register_system(
		std::uint32_t precedence, const System *system
	) noexcept;

	void tick(World &world) const noexcept;

	struct Registar {
		Registar(std::uint32_t precedence, const System *system);
	};

private:
	std::vector<std::tuple<std::uint32_t, const System *>> systems_;
};

} // namespace istd

#endif