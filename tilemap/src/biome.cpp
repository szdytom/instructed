#include "biome.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace istd {

// Biome properties lookup table
constexpr std::array<BiomeProperties, 9> biome_properties = {
	{// Desert: Hot & Dry
     {.water_threshold = 0.1,
      .sand_threshold = 0.7,
      .wood_threshold = 0.85,
      .mountain_threshold = 0.9,
      .scale = 0.03,
      .octaves = 3,
      .persistence = 0.4,
      .name = "Desert"},
     // Savanna: Hot & Moderate
     {.water_threshold = 0.15,
      .sand_threshold = 0.3,
      .wood_threshold = 0.75,
      .mountain_threshold = 0.88,
      .scale = 0.025,
      .octaves = 4,
      .persistence = 0.5,
      .name = "Savanna"},
     // TropicalRainforest: Hot & Wet
     {.water_threshold = 0.25,
      .sand_threshold = 0.35,
      .wood_threshold = 0.8,
      .mountain_threshold = 0.9,
      .scale = 0.02,
      .octaves = 5,
      .persistence = 0.6,
      .name = "Tropical Rainforest"},
     // Grassland: Temperate & Dry
     {.water_threshold = 0.2,
      .sand_threshold = 0.4,
      .wood_threshold = 0.7,
      .mountain_threshold = 0.85,
      .scale = 0.035,
      .octaves = 3,
      .persistence = 0.45,
      .name = "Grassland"},
     // DeciduousForest: Temperate & Moderate
     {.water_threshold = 0.3,
      .sand_threshold = 0.4,
      .wood_threshold = 0.75,
      .mountain_threshold = 0.87,
      .scale = 0.025,
      .octaves = 4,
      .persistence = 0.55,
      .name = "Deciduous Forest"},
     // TemperateRainforest: Temperate & Wet
     {.water_threshold = 0.35,
      .sand_threshold = 0.45,
      .wood_threshold = 0.8,
      .mountain_threshold = 0.9,
      .scale = 0.02,
      .octaves = 5,
      .persistence = 0.6,
      .name = "Temperate Rainforest"},
     // Tundra: Cold & Dry
     {.water_threshold = 0.15,
      .sand_threshold = 0.25,
      .wood_threshold = 0.5,
      .mountain_threshold = 0.8,
      .scale = 0.04,
      .octaves = 2,
      .persistence = 0.3,
      .name = "Tundra"},
     // Taiga: Cold & Moderate
     {.water_threshold = 0.25,
      .sand_threshold = 0.35,
      .wood_threshold = 0.75,
      .mountain_threshold = 0.85,
      .scale = 0.03,
      .octaves = 4,
      .persistence = 0.5,
      .name = "Taiga"},
     // ColdRainforest: Cold & Wet
     {.water_threshold = 0.3,
      .sand_threshold = 0.4,
      .wood_threshold = 0.8,
      .mountain_threshold = 0.9,
      .scale = 0.025,
      .octaves = 5,
      .persistence = 0.6,
      .name = "Cold Rainforest"}
    }
};

const BiomeProperties &get_biome_properties(BiomeType biome) {
	return biome_properties[static_cast<std::uint8_t>(biome)];
}

BiomeType determine_biome(double temperature, double humidity) {
	// Normalize temperature and humidity to 0-1 range if needed
	temperature = std::clamp(temperature, 0.0, 1.0);
	humidity = std::clamp(humidity, 0.0, 1.0);

	// Determine temperature category
	int temp_category;
	if (temperature < 0.33) {
		temp_category = 0; // Cold
	} else if (temperature < 0.67) {
		temp_category = 1; // Temperate
	} else {
		temp_category = 2; // Hot
	}

	// Determine humidity category
	int humidity_category;
	if (humidity < 0.33) {
		humidity_category = 0; // Dry
	} else if (humidity < 0.67) {
		humidity_category = 1; // Moderate
	} else {
		humidity_category = 2; // Wet
	}

	// Map to biome type (3x3 grid)
	// Cold (0): Tundra, Taiga, ColdRainforest
	// Temperate (1): Grassland, DeciduousForest, TemperateRainforest
	// Hot (2): Desert, Savanna, TropicalRainforest

	static constexpr BiomeType biome_matrix[3][3] = {
		// Cold row
		{BiomeType::Tundra,    BiomeType::Taiga,           BiomeType::ColdRainforest    },
		// Temperate row
		{BiomeType::Grassland, BiomeType::DeciduousForest,
	     BiomeType::TemperateRainforest												 },
		// Hot row
		{BiomeType::Desert,    BiomeType::Savanna,         BiomeType::TropicalRainforest}
	};

	return biome_matrix[temp_category][humidity_category];
}

} // namespace istd
