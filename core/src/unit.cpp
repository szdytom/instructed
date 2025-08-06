#include "istd_core/unit.h"
#include "istd_core/system.h"
#include "istd_util/tile_geometry.h"
#include "tilemap/tile.h"

namespace istd {

namespace {

struct ResetVelocitySystem : public System {
	void tick(World &world) const noexcept override {
		auto &reg = world.registry;
		reg.view<KinematicsComponent>().each(
			[](entt::entity, KinematicsComponent &kinematics) {
			kinematics.velocity = Vec2::zero();
		}
		);
	}

	std::string_view name() const noexcept override {
		return "Reset Velocity System";
	}
};

static const ResetVelocitySystem reset_velocity_system;
static const SystemRegistry::Registar reset_velocity_registrar(
	System::Precedence::ResetVelocity, &reset_velocity_system
);

bool is_passible_tile(Tile tile) {
	return tile.base != BaseTileType::Mountain;
}

void update_pos(const TileMap &tilemap, KinematicsComponent &kinematics) {
	auto next_pos = kinematics.position + kinematics.velocity;
	auto ray_cast = tiles_on_segment(kinematics.position, next_pos);
	for (auto [i, j] : ray_cast) {
		auto tile = tilemap.get_tile(TilePos::from_global(i, j));
		if (!is_passible_tile(tile)) {
			// Hit an impassable tile
			// Calculate intersection point
			auto inter = tile_segment_intersection(
				kinematics.position, next_pos, {i, j}
			);
			next_pos = inter;
			break;
		}
	}
	kinematics.position = next_pos;
}

struct KinematicsSystem : public System {
	void tick(World &world) const noexcept override {
		auto &reg = world.registry;
		reg.view<KinematicsComponent>().each(
			std::bind(
				update_pos, std::ref(world.tilemap), std::placeholders::_1
			)
		);
	}

	std::string_view name() const noexcept override {
		return "Kinematics System";
	}
};

static const KinematicsSystem kinematics_system;
static const SystemRegistry::Registar kinematics_registrar(
	System::Precedence::UpdateKinematics, &kinematics_system
);

} // namespace

} // namespace istd