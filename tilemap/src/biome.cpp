#include "biome.h"
#include "chunk.h"
#include <algorithm>

namespace istd {

// Biome properties lookup table
constexpr BiomeProperties biome_properties[] = {
	// Snowy Peeks (Cold & Dry)
	{
     .name = "Snowy Peeks",
     .temperature = BiomeTemperature::Cold,
     .humidity = BiomeHumidity::Dry,
     .is_ocean = false,
     .water_ratio = .05,
     .ice_ratio = .2,
     .sand_ratio = .05,
     .land_ratio = .3,
	 },
	// Snowy Plains (Cold & Moderate)
	{
     .name = "Snowy Plains",
     .temperature = BiomeTemperature::Cold,
     .humidity = BiomeHumidity::Moderate,
     .is_ocean = false,
     .water_ratio = .05,
     .ice_ratio = .25,
     .sand_ratio = .1,
     .land_ratio = .4,
	 },
	// Frozen Ocean (Cold & Wet)
	{
     .name = "Frozen Ocean",
     .temperature = BiomeTemperature::Cold,
     .humidity = BiomeHumidity::Wet,
     .is_ocean = true,
     .water_ratio = .15,
     .ice_ratio = .8,
     .sand_ratio = .05,
     .land_ratio = .0,
	 },
	// Plains (Temperate & Dry)
	{
     .name = "Plains",
     .temperature = BiomeTemperature::Temperate,
     .humidity = BiomeHumidity::Dry,
     .is_ocean = false,
     .water_ratio = .05,
     .ice_ratio = .0,
     .sand_ratio = .05,
     .land_ratio = .7,
	 },
	// Forest (Temperate & Moderate)
	{
     .name = "Forest",
     .temperature = BiomeTemperature::Temperate,
     .humidity = BiomeHumidity::Moderate,
     .is_ocean = false,
     .water_ratio = .1,
     .ice_ratio = .0,
     .sand_ratio = .05,
     .land_ratio = .75,
	 },
	// Ocean (Temperate & Wet)
	{
     .name = "Ocean",
     .temperature = BiomeTemperature::Temperate,
     .humidity = BiomeHumidity::Wet,
     .is_ocean = true,
     .water_ratio = .95,
     .ice_ratio = .0,
     .sand_ratio = .03,
     .land_ratio = .02,
	 },
	// Desert (Hot & Dry)
	{
     .name = "Desert",
     .temperature = BiomeTemperature::Hot,
     .humidity = BiomeHumidity::Dry,
     .is_ocean = false,
     .water_ratio = .0,
     .ice_ratio = .0,
     .sand_ratio = .85,
     .land_ratio = .0,
	 },
	// Savanna (Hot & Moderate)
	{
     .name = "Savanna",
     .temperature = BiomeTemperature::Hot,
     .humidity = BiomeHumidity::Moderate,
     .is_ocean = false,
     .water_ratio = .2,
     .ice_ratio = .0,
     .sand_ratio = .1,
     .land_ratio = .5,
	 },
	// Luke Ocean (Hot & Wet)
	{
     .name = "Luke Ocean",
     .temperature = BiomeTemperature::Hot,
     .humidity = BiomeHumidity::Wet,
     .is_ocean = true,
     .water_ratio = .95,
     .ice_ratio = .0,
     .sand_ratio = .01,
     .land_ratio = .04,
	 },
};

const BiomeProperties &get_biome_properties(BiomeType biome) {
	return biome_properties[static_cast<std::uint8_t>(biome)];
}

BiomeType determine_biome(double temperature, double humidity) {
	// Normalize temperature and humidity to 0-1 range if needed
	temperature = std::clamp(temperature, 0.0, 1.0);
	humidity = std::clamp(humidity, 0.0, 1.0);

	const double threshold1 = 0.33;
	const double threshold2 = 0.67;

	BiomeTemperature temp_category;
	if (temperature < threshold1) {
		temp_category = BiomeTemperature::Cold;
	} else if (temperature < threshold2) {
		temp_category = BiomeTemperature::Temperate;
	} else {
		temp_category = BiomeTemperature::Hot;
	}

	BiomeHumidity humidity_category;
	if (humidity < threshold1) {
		humidity_category = BiomeHumidity::Dry;
	} else if (humidity < threshold2) {
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
