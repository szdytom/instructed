#include "istd_core/world.h"
#include "istd_core/system.h"
#include "tilemap/generation.h"

namespace istd {

namespace {

struct TickSystem : public System {
	void tick(World &world) const noexcept override {
		world.tick += 1;
	}

	std::string_view name() const noexcept override {
		return "Tick System";
	}
};

static const TickSystem tick_system;
static const SystemRegistry::Registar tick_registrar(
	System::Precedence::Highest, &tick_system
);

} // namespace

World::World(std::uint8_t size)
	: tick(0), tilemap(size), rooms(size, std::vector<Room>(size, {0, 0})) {
	for (std::uint8_t x = 0; x < size; ++x) {
		for (std::uint8_t y = 0; y < size; ++y) {
			rooms[x][y] = {x, y};
		}
	}
}

void World::generateTilemap(const GenerationConfig &config) {
	map_generate(tilemap, config);
}

} // namespace istd
