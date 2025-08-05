#ifndef ISTD_CORE_DEVICE_H
#define ISTD_CORE_DEVICE_H

#include "entt/entity/fwd.hpp"
#include "istd_core/item.h"
#include "istd_core/world.h"
#include "istd_util/small_map.h"
#include <cstdint>
#include <entt/entt.hpp>
#include <string_view>

namespace istd {

using ItemPort = std::int16_t;
using DeviceId = std::uint8_t;

// device's register
class RegSetStrategy {
public:
	virtual bool read(
		World &world, entt::entity entity, std::uint8_t reg_id,
		std::uint32_t &value
	) const noexcept;

	virtual bool write(
		World &world, entt::entity entity, std::uint8_t reg_id,
		std::uint32_t value
	) const noexcept;
};

struct DevicePrototype {
	std::string_view name;
	RegSetStrategy *reg_set_strategy; // life cycle: static
	std::uint32_t mass;
	ItemPort input_n, output_n;       // number of input/output ports
};

struct DevicePrototypeComponent {
	DevicePrototype *prototype; // life cycle: static
};

struct DeviceIdComponent {
	entt::entity unit;      // the unit this device belongs to
	std::uint8_t device_id; // unique ID within the unit
};

struct DeviceBuilder {
	virtual entt::entity build(
		World &world, entt::entity unit, DeviceId device_id
	) const
		= 0;
};

struct DeviceBuilderRegistry {
	static DeviceBuilderRegistry &instance();

	void register_builder(Item item, DeviceBuilder *builder);
	entt::entity build(
		World &world, Item item, entt::entity unit, DeviceId device_id
	) const;

private:
	SmallMap<Item, DeviceBuilder *> builders_;
};

} // namespace istd

#endif
