#include "istd_core/devices/vehicle.h"
#include "istd_core/device.h"
#include "istd_core/system.h"
#include "istd_core/unit.h"

namespace istd {
namespace {

// Vehicle register allocations:
// 0: Vehicle speed (1 = 1 / 256 of max speed)
// 1: Vehicle heading (1 = 1 / 256 pi)
// 2: Vehicle x position (1 = 1 / 1024 tile) (read-only)
// 3: Vehicle y position (1 = 1 / 1024 tile) (read-only)
// 4: Device status / error code

struct VehicleRegSetStrategy : public RegSetStrategy {
	virtual bool read(
		World &world, entt::entity entity, std::uint8_t reg_id,
		std::uint32_t &value
	) const noexcept override {
		auto &reg = world.registry;
		switch (reg_id) {
		case 0: { // Vehicle speed
			auto &vehicle = reg.get<const VehicleComponent>(entity);
			value = static_cast<std::uint32_t>(vehicle.speed * 256.0f);
			break;
		}

		case 1: { // Vehicle heading
			auto &vehicle = reg.get<const VehicleComponent>(entity);
			value = static_cast<std::uint32_t>(
				(vehicle.heading / (2.0f * M_PI)) * 256.0f
			);
			break;
		}

		case 2: { // Vehicle x position
			auto owner = reg.get<const DeviceIdComponent>(entity).unit;
			auto pos = reg.get<const KinematicsComponent>(owner).position;
			value = static_cast<std::uint32_t>(pos.x * 1024);
			break;
		}

		case 3: { // Vehicle y position
			auto owner = reg.get<const DeviceIdComponent>(entity).unit;
			auto pos = reg.get<const KinematicsComponent>(owner).position;
			value = static_cast<std::uint32_t>(pos.y * 1024);
			break;
		}

		case 4:
			// No error code for now, just return 0
			value = 0;
			break;

		default:
			return false; // Invalid register ID
		}
		return true;      // Read successful
	}

	virtual bool write(
		World &world, entt::entity entity, std::uint8_t reg_id,
		std::uint32_t value
	) const noexcept override {
		switch (reg_id) {
		case 0: { // Vehicle speed
			auto &vehicle = world.registry.get<VehicleComponent>(entity);
			vehicle.speed = static_cast<float>(value) / 256.0f;
			return true;
		}
		case 1: { // Vehicle heading
			auto &vehicle = world.registry.get<VehicleComponent>(entity);
			vehicle.heading = (static_cast<float>(value) / 256.0f)
				* (2.0f * M_PI);
			return true;
		}
		default:
			return false; // Invalid register ID or read-only register
		}
	}
};

static const VehicleRegSetStrategy vehicle_reg_set_strategy;

static const VehiclePrototype basic_vihicle_prototype = {
	.name = "Basic Vehicle",
	.ideal_working_mass = 7440,
	.max_speed = 0.3f, // 1 tile per tick
};

static const DevicePrototype basic_vehicle_device_prototype = {
	.name = "Basic Vehicle Device",
	.reg_set_strategy = &vehicle_reg_set_strategy,
	.mass = 3270,
	.item = ItemType::BasicVehicleChassis,
	.input_n = 0,
	.output_n = 0,
};

struct BasicVehicleBuilder : public DeviceBuilder {
	entt::entity build(
		World &world, entt::entity unit, DeviceId device_id
	) const override {
		auto entity = world.registry.create();

		world.registry.emplace<DevicePrototypeComponent>(
			entity, &basic_vehicle_device_prototype
		);

		world.registry.emplace<VehicleComponent>(
			entity, &basic_vihicle_prototype, 0.0f, 0.0f
		);

		return entity;
	}
};

static const BasicVehicleBuilder basic_vehicle_builder;
static const DeviceBuilderRegistry::Registar basic_vehicle_registrar(
	ItemType::BasicVehicleChassis, &basic_vehicle_builder
);

struct VehicleVelocitySystem : public System {
	void tick(World &world) const noexcept override {
		auto &reg = world.registry;
		reg.view<const VehicleComponent, const DeviceIdComponent>().each(
			[&reg](
				entt::entity entity, const VehicleComponent &vehicle,
				const DeviceIdComponent &device_id
			) {
			// Update vehicle position based on speed and heading
			auto owner = device_id.unit;
			if (!reg.all_of<OnGroundFlag>(owner)) {
				// If not on ground, do not update position
				return;
			}

			auto &kinematics = reg.get<KinematicsComponent>(owner);
			kinematics.velocity += Vec2::rotated(
				vehicle.heading, vehicle.speed * vehicle.prototype->max_speed
			);
		}
		);
	}

	std::string_view name() const noexcept override {
		return "Vehicle Device System";
	}
};

static const VehicleVelocitySystem vehicle_velocity_system;
static const SystemRegistry::Registar vehicle_velocity_registrar(
	System::Precedence::DeviceAccumulateVelocity, &vehicle_velocity_system
);

} // namespace

} // namespace istd