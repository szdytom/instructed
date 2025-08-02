#include "generation.h"

namespace istd {
TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config), master_rng_(config.seed) {}

void TerrainGenerator::operator()(TileMap &tilemap) {
	biome_pass(tilemap);
	base_tile_type_pass(tilemap);
	smoothen_mountains_pass(tilemap);
	smoothen_islands_pass(tilemap);
	mountain_hole_fill_pass(tilemap);
	deepwater_pass(tilemap);
}

void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator(tilemap);
}

} // namespace istd
