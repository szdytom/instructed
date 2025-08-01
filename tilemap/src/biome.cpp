#include "biome.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace istd {

// Biome properties lookup table
constexpr BiomeProperties biome_properties[] = {
	// Snowy Peeks (Cold & Dry)
	{
     .name = "Snowy Peeks",

     .water_ratio = .05,
     .ice_ratio = .15,
     .sand_ratio = .1,
     .land_ratio = .2,
	 },
	// Snowy Plains (Cold & Moderate)
	{
     .name = "Snowy Plains",
     .water_ratio = .05,
     .ice_ratio = .25,
     .sand_ratio = .1,
     .land_ratio = .4,
	 },
	// Frozen Ocean (Cold & Wet)
	{
     .name = "Frozen Ocean",
     .water_ratio = .3,
     .ice_ratio = .4,
     .sand_ratio = .25,
     .land_ratio = .05,
	 },
	// Plains (Temperate & Dry)
	{
     .name = "Plains",
     .water_ratio = .1,
     .ice_ratio = .0,
     .sand_ratio = .05,
     .land_ratio = .65,
	 },
	// Forest (Temperate & Moderate)
	{
     .name = "Forest",
     .water_ratio = .2,
     .ice_ratio = .0,
     .sand_ratio = .1,
     .land_ratio = .5,
	 },
	// Ocean (Temperate & Wet)
	{
     .name = "Ocean",
     .water_ratio = .7,
     .ice_ratio = .0,
     .sand_ratio = .2,
     .land_ratio = .1,
	 },
	// Desert (Hot & Dry)
	{
     .name = "Desert",
     .water_ratio = .0,
     .ice_ratio = .0,
     .sand_ratio = .75,
     .land_ratio = .05,
	 },
	// Savanna (Hot & Moderate)
	{
     .name = "Savanna",
     .water_ratio = .2,
     .ice_ratio = .0,
     .sand_ratio = .1,
     .land_ratio = .5,
	 },
	// Luke Ocean (Hot & Wet)
	{
     .name = "Luke Ocean",
     .water_ratio = .8,
     .ice_ratio = .0,
     .sand_ratio = .2,
     .land_ratio = .0,
	 },
};

const BiomeProperties &get_biome_properties(BiomeType biome) {
	return biome_properties[static_cast<std::uint8_t>(biome)];
}

BiomeType determine_biome(double temperature, double humidity) {
	// Normalize temperature and humidity to 0-1 range if needed
	temperature = std::clamp(temperature, 0.0, 1.0);
	humidity = std::clamp(humidity, 0.0, 1.0);

	BiomeTemperature temp_category;
	if (temperature < 0.33) {
		temp_category = BiomeTemperature::Cold;
	} else if (temperature < 0.66) {
		temp_category = BiomeTemperature::Temperate;
	} else {
		temp_category = BiomeTemperature::Hot;
	}

	BiomeHumidity humidity_category;
	if (humidity < 0.33) {
		humidity_category = BiomeHumidity::Dry;
	} else if (humidity < 0.66) {
		humidity_category = BiomeHumidity::Moderate;
	} else {
		humidity_category = BiomeHumidity::Wet;
	}

	int mat_x = static_cast<int>(temp_category);
	int mat_y = static_cast<int>(humidity_category);
	int index = mat_x * 3 + mat_y; // 3 humidity categories
	return static_cast<BiomeType>(index);
}

} // namespace istd
