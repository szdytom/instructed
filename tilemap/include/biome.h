#ifndef ISTD_TILEMAP_BIOME_H
#define ISTD_TILEMAP_BIOME_H

#include <cstdint>
#include <string_view>

namespace istd {

// Biome types based on temperature and humidity
enum class BiomeType : std::uint8_t {
	Desert = 0,              // Hot & Dry
	Savanna = 1,             // Hot & Moderate
	TropicalRainforest = 2,  // Hot & Wet
	Grassland = 3,           // Temperate & Dry
	DeciduousForest = 4,     // Temperate & Moderate
	TemperateRainforest = 5, // Temperate & Wet
	Tundra = 6,              // Cold & Dry
	Taiga = 7,               // Cold & Moderate
	ColdRainforest = 8       // Cold & Wet
};

// Biome properties for terrain generation
struct BiomeProperties {
	// Terrain thresholds (0.0 - 1.0)
	double water_threshold;
	double sand_threshold;
	double wood_threshold;
	double mountain_threshold;

	// Noise parameters
	double scale;
	int octaves;
	double persistence;

	// Biome name for debugging
	std::string_view name;
};

// Get biome properties for terrain generation
const BiomeProperties &get_biome_properties(BiomeType biome);

// Determine biome type based on temperature and humidity
BiomeType determine_biome(double temperature, double humidity);

// Sub-chunk position within a chunk (4x4 grid of 16x16 sub-chunks)
struct SubChunkPos {
	std::uint8_t sub_x; // 0-3
	std::uint8_t sub_y; // 0-3

	constexpr SubChunkPos(std::uint8_t x, std::uint8_t y): sub_x(x), sub_y(y) {}
};

// Convert local tile coordinates to sub-chunk position
constexpr SubChunkPos tile_to_subchunk(
	std::uint8_t local_x, std::uint8_t local_y
) {
	return SubChunkPos(local_x / 16, local_y / 16);
}

// Get the starting tile coordinates for a sub-chunk
constexpr std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(
	const SubChunkPos &pos
) {
	return {pos.sub_x * 16, pos.sub_y * 16};
}

} // namespace istd

#endif
