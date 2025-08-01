#include "biome.h"
#include <array>
#include <cmath>
#include <algorithm>

namespace istd {

// Biome properties lookup table
constexpr std::array<BiomeProperties, 9> biome_properties = {{
    // Desert: Hot & Dry
    {
        .water_threshold = 0.1,
        .mountain_threshold = 0.85,
        .sand_threshold = 0.8,
        .ice_threshold = 0.0,  // No ice in desert
        .wood_threshold = 0.05,  // Very sparse vegetation
        .snow_threshold = 0.0,   // No snow in desert
        .base_scale = 0.03,
        .base_octaves = 3,
        .base_persistence = 0.4,
        .surface_scale = 0.05,
        .surface_octaves = 2,
        .surface_persistence = 0.3,
        .name = "Desert"
    },
    // Savanna: Hot & Moderate
    {
        .water_threshold = 0.15,
        .mountain_threshold = 0.8,
        .sand_threshold = 0.3,
        .ice_threshold = 0.0,
        .wood_threshold = 0.25,  // Scattered trees
        .snow_threshold = 0.0,
        .base_scale = 0.025,
        .base_octaves = 4,
        .base_persistence = 0.5,
        .surface_scale = 0.04,
        .surface_octaves = 3,
        .surface_persistence = 0.4,
        .name = "Savanna"
    },
    // TropicalRainforest: Hot & Wet
    {
        .water_threshold = 0.25,
        .mountain_threshold = 0.85,
        .sand_threshold = 0.1,
        .ice_threshold = 0.0,
        .wood_threshold = 0.7,   // Dense forest
        .snow_threshold = 0.0,
        .base_scale = 0.02,
        .base_octaves = 5,
        .base_persistence = 0.6,
        .surface_scale = 0.03,
        .surface_octaves = 4,
        .surface_persistence = 0.5,
        .name = "Tropical Rainforest"
    },
    // Grassland: Temperate & Dry
    {
        .water_threshold = 0.2,
        .mountain_threshold = 0.8,
        .sand_threshold = 0.15,
        .ice_threshold = 0.0,
        .wood_threshold = 0.15,  // Sparse trees
        .snow_threshold = 0.05,  // Occasional snow
        .base_scale = 0.035,
        .base_octaves = 3,
        .base_persistence = 0.45,
        .surface_scale = 0.06,
        .surface_octaves = 2,
        .surface_persistence = 0.35,
        .name = "Grassland"
    },
    // DeciduousForest: Temperate & Moderate
    {
        .water_threshold = 0.3,
        .mountain_threshold = 0.82,
        .sand_threshold = 0.05,
        .ice_threshold = 0.0,
        .wood_threshold = 0.6,   // Dense deciduous forest
        .snow_threshold = 0.1,   // Some snow coverage
        .base_scale = 0.025,
        .base_octaves = 4,
        .base_persistence = 0.55,
        .surface_scale = 0.04,
        .surface_octaves = 3,
        .surface_persistence = 0.45,
        .name = "Deciduous Forest"
    },
    // TemperateRainforest: Temperate & Wet
    {
        .water_threshold = 0.35,
        .mountain_threshold = 0.85,
        .sand_threshold = 0.05,
        .ice_threshold = 0.0,
        .wood_threshold = 0.8,   // Very dense forest
        .snow_threshold = 0.15,  // More snow
        .base_scale = 0.02,
        .base_octaves = 5,
        .base_persistence = 0.6,
        .surface_scale = 0.03,
        .surface_octaves = 4,
        .surface_persistence = 0.5,
        .name = "Temperate Rainforest"
    },
    // Tundra: Cold & Dry
    {
        .water_threshold = 0.15,
        .mountain_threshold = 0.75,
        .sand_threshold = 0.05,
        .ice_threshold = 0.3,    // Lots of ice
        .wood_threshold = 0.05,  // Very sparse vegetation
        .snow_threshold = 0.4,   // Lots of snow
        .base_scale = 0.04,
        .base_octaves = 2,
        .base_persistence = 0.3,
        .surface_scale = 0.08,
        .surface_octaves = 2,
        .surface_persistence = 0.25,
        .name = "Tundra"
    },
    // Taiga: Cold & Moderate
    {
        .water_threshold = 0.25,
        .mountain_threshold = 0.8,
        .sand_threshold = 0.02,
        .ice_threshold = 0.15,
        .wood_threshold = 0.5,   // Coniferous forest
        .snow_threshold = 0.6,   // Heavy snow coverage
        .base_scale = 0.03,
        .base_octaves = 4,
        .base_persistence = 0.5,
        .surface_scale = 0.05,
        .surface_octaves = 3,
        .surface_persistence = 0.4,
        .name = "Taiga"
    },
    // FrozenOcean: Cold & Wet
    {
        .water_threshold = 0.4,
        .mountain_threshold = 0.85,
        .sand_threshold = 0.02,
        .ice_threshold = 0.6,    // Mostly ice
        .wood_threshold = 0.1,   // Very sparse trees
        .snow_threshold = 0.8,   // Almost all snow
        .base_scale = 0.025,
        .base_octaves = 3,
        .base_persistence = 0.4,
        .surface_scale = 0.04,
        .surface_octaves = 2,
        .surface_persistence = 0.3,
        .name = "Frozen Ocean"
    }
}};

const BiomeProperties& get_biome_properties(BiomeType biome) {
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
    static constexpr BiomeType biome_matrix[3][3] = {
        // Cold row
        {BiomeType::Tundra, BiomeType::Taiga, BiomeType::FrozenOcean},
        // Temperate row
        {BiomeType::Grassland, BiomeType::DeciduousForest, BiomeType::TemperateRainforest},
        // Hot row
        {BiomeType::Desert, BiomeType::Savanna, BiomeType::TropicalRainforest}
    };
    
    return biome_matrix[temp_category][humidity_category];
}

} // namespace istd
