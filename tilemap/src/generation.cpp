#include "tilemap/generation.h"

namespace istd {
TerrainGenerator::TerrainGenerator(const GenerationConfig &config)
	: config_(config) {}

void TerrainGenerator::operator()(TileMap &tilemap) {
	master_rng_ = Xoroshiro128PP(config_.seed);
	biome_pass(tilemap);
	base_tile_type_pass(tilemap);
	smoothen_mountains_pass(tilemap);
	smoothen_islands_pass(tilemap);
	mountain_hole_fill_pass(tilemap);
	deepwater_pass(tilemap);
	oil_pass(tilemap);
	mineral_cluster_pass(tilemap);
	coal_pass(tilemap);
}

void map_generate(TileMap &tilemap, const GenerationConfig &config) {
	TerrainGenerator generator(config);
	generator(tilemap);
}

} // namespace istd
