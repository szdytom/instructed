#include "istd_core/system.h"
#include <algorithm>

namespace istd {

SystemRegistry &SystemRegistry::instance() noexcept {
	static SystemRegistry registry;
	return registry;
}

void SystemRegistry::register_system(
	std::uint32_t precedence, const System *system
) noexcept {
	auto cmp = [](const std::tuple<std::uint32_t, const System *> &a,
	              std::uint32_t b) {
		return std::get<0>(a) < b;
	};

	auto it = std::lower_bound(
		systems_.begin(), systems_.end(), precedence, cmp
	);

	systems_.insert(it, std::make_tuple(precedence, system));
}

void SystemRegistry::tick(World &world) const noexcept {
	for (const auto &[_, system] : systems_) {
		system->tick(world);
	}
}

SystemRegistry::Registar::Registar(
	std::uint32_t precedence, const System *system
) {
	SystemRegistry::instance().register_system(precedence, system);
}

} // namespace istd