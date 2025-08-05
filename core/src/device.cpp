#include "istd_core/device.h"
#include "istd_core/world.h"

namespace istd {

bool RegSetStrategy::read(
	World &, entt::entity, std::uint8_t, std::uint32_t &
) const noexcept {
	return false;
}

bool RegSetStrategy::write(
	World &, entt::entity, std::uint8_t, std::uint32_t
) const noexcept {
	return false;
}

DeviceBuilderRegistry &DeviceBuilderRegistry::instance() {
	static DeviceBuilderRegistry registry;
	return registry;
}

void DeviceBuilderRegistry::register_builder(
	Item item, DeviceBuilder *builder
) {
	builders_.insert(item, builder);
}

entt::entity DeviceBuilderRegistry::build(
	World &world, Item item, entt::entity unit, DeviceId device_id
) const {
	auto builder = builders_[item];
	return builder->build(world, unit, device_id);
}

} // namespace istd
