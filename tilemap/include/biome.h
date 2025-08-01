#ifndef TILEMAP_BIOME_H
#define TILEMAP_BIOME_H

#include <cstdint>
#include <string_view>

namespace istd {

// Biome types based on temperature and humidity
enum class BiomeType : std::uint8_t {
	SnowyPeeks = 0,  // Cold & Dry
	SnowyPlains = 1, // Cold & Moderate
	FrozenOcean = 2, // Cold & Wet
	Plains = 3,      // Temperate & Dry
	Forest = 4,      // Temperate & Moderate
	Ocean = 5,       // Temperate & Wet
	Desert = 6,      // Hot & Dry
	Savanna = 7,     // Hot & Moderate
	LukeOcean = 8,   // Hot & Wet
};

enum class BiomeTemperature : std::uint8_t {
	Cold = 0,
	Temperate = 1,
	Hot = 2,
};

enum class BiomeHumidity : std::uint8_t {
	Dry = 0,
	Moderate = 1,
	Wet = 2,
};

// Biome properties for terrain generation
struct BiomeProperties {
	// Biome name for debugging
	std::string_view name;

	// Base terrain ratios (0.0 - 1.0)
	double water_ratio;
	double ice_ratio;
	double sand_ratio;
	double land_ratio;
};

// Get biome properties for terrain generation
const BiomeProperties &get_biome_properties(BiomeType biome);

// Determine biome type based on temperature and humidity
BiomeType determine_biome(double temperature, double humidity);

} // namespace istd

#endif
