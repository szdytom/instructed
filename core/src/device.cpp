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

DeviceBuilderRegistry &DeviceBuilderRegistry::instance() noexcept {
	static DeviceBuilderRegistry registry;
	return registry;
}

void DeviceBuilderRegistry::register_builder(
	ItemType item, const DeviceBuilder *builder
) {
	builders_.insert(item, builder);
}

entt::entity DeviceBuilderRegistry::build(
	World &world, ItemType item, entt::entity unit, DeviceId device_id
) const {
	auto builder = builders_[item];
	auto entity = builder->build(world, unit, device_id);
	world.registry.emplace<DeviceIdComponent>(entity, unit, device_id);
	return entity;
}

DeviceBuilderRegistry::Registar::Registar(
	ItemType item, const DeviceBuilder *builder
) {
	DeviceBuilderRegistry::instance().register_builder(item, builder);
}

} // namespace istd
