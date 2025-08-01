#include "biome.h"
#include "generation.h"
#include "tilemap.h"
#include <iomanip>
#include <iostream>

using namespace istd;

// Function to get character representation for biome visualization
char get_biome_char(BiomeType biome) {
	switch (biome) {
	case BiomeType::Desert:
		return 'D';
	case BiomeType::Savanna:
		return 'S';
	case BiomeType::TropicalRainforest:
		return 'T';
	case BiomeType::Grassland:
		return 'G';
	case BiomeType::DeciduousForest:
		return 'F';
	case BiomeType::TemperateRainforest:
		return 'R';
	case BiomeType::Tundra:
		return 'U';
	case BiomeType::Taiga:
		return 'A';
	case BiomeType::FrozenOcean:
		return 'O';
	}
	return '?';
}

// Function to get biome name as string
const char *get_biome_name(BiomeType biome) {
	switch (biome) {
	case BiomeType::Desert:
		return "Desert";
	case BiomeType::Savanna:
		return "Savanna";
	case BiomeType::TropicalRainforest:
		return "Tropical Rainforest";
	case BiomeType::Grassland:
		return "Grassland";
	case BiomeType::DeciduousForest:
		return "Deciduous Forest";
	case BiomeType::TemperateRainforest:
		return "Temperate Rainforest";
	case BiomeType::Tundra:
		return "Tundra";
	case BiomeType::Taiga:
		return "Taiga";
	case BiomeType::FrozenOcean:
		return "Frozen Ocean";
	}
	return "Unknown";
}

int main() {
	std::cout << "=== Biome System Demo ===" << std::endl;
	std::cout << "This demo shows biome distribution based on temperature and "
	             "humidity"
			  << std::endl;
	std::cout << "Legend:" << std::endl;
	std::cout << "  D = Desert        S = Savanna       T = Tropical"
			  << std::endl;
	std::cout << "  G = Grassland     F = Deciduous     R = Temp.Rain"
			  << std::endl;
	std::cout << "  U = Tundra        A = Taiga         O = Frozen"
			  << std::endl;
	std::cout << std::endl;

	// Create a tilemap for biome demonstration
	constexpr std::uint8_t map_size = 3; // 3x3 chunks for compact display
	TileMap tilemap(map_size);

	// Configure generation with good biome variety
	GenerationConfig config;
	config.seed = 98765;
	config.temperature_scale = 0.003; // Larger temperature zones
	config.humidity_scale = 0.004;    // Larger humidity zones

	std::cout << "Generating " << static_cast<int>(map_size) << "x"
			  << static_cast<int>(map_size) << " chunks..." << std::endl;

	// Generate the map
	map_generate(tilemap, config);

	std::cout << "Generation complete!" << std::endl << std::endl;

	// Show biome properties
	std::cout << "=== Biome Properties ===" << std::endl;
	const BiomeType all_biomes[]
		= {BiomeType::Desert,
	       BiomeType::Savanna,
	       BiomeType::TropicalRainforest,
	       BiomeType::Grassland,
	       BiomeType::DeciduousForest,
	       BiomeType::TemperateRainforest,
	       BiomeType::Tundra,
	       BiomeType::Taiga,
	       BiomeType::FrozenOcean};

	for (BiomeType biome : all_biomes) {
		const BiomeProperties &props = get_biome_properties(biome);
		std::cout << get_biome_name(biome) << " (" << get_biome_char(biome)
				  << "):" << std::endl;
		std::cout << "  Base terrain - Scale: " << props.base_scale
				  << " Octaves: " << props.base_octaves
				  << " Persistence: " << props.base_persistence << std::endl;
		std::cout << "  Surface features - Scale: " << props.surface_scale
				  << " Octaves: " << props.surface_octaves
				  << " Persistence: " << props.surface_persistence << std::endl;
		std::cout << "  Thresholds - Water: " << props.water_threshold
				  << " Sand: " << props.sand_threshold
				  << " Mountain: " << props.mountain_threshold << std::endl;
		std::cout << "  Features - Wood: " << props.wood_threshold
				  << " Snow: " << props.snow_threshold << std::endl;
		std::cout << std::endl;
	}

	// Sample some actual generated tiles to show the system working
	std::cout << "=== Sample Generated Tiles ===" << std::endl;
	for (int i = 0; i < 9; ++i) {
		std::uint8_t chunk_x = i % map_size;
		std::uint8_t chunk_y = i / map_size;
		std::uint8_t local_x = 32; // Middle of chunk
		std::uint8_t local_y = 32;

		TilePos pos{chunk_x, chunk_y, local_x, local_y};
		Tile tile = tilemap.get_tile(pos);

		std::cout << "Chunk (" << static_cast<int>(chunk_x) << ","
				  << static_cast<int>(chunk_y) << "): ";

		// Convert base and surface types to readable strings
		const char *base_names[] = {"Land", "Mountain", "Sand", "Water", "Ice"};
		const char *surface_names[] = {"Empty", "Wood", "Snow"};

		std::cout << "Base=" << base_names[static_cast<int>(tile.base)]
				  << " Surface="
				  << surface_names[static_cast<int>(tile.surface)] << std::endl;
	}

	// Show climate zones demonstration
	std::cout << std::endl << "=== Climate Zone Demonstration ===" << std::endl;
	std::cout
		<< "Temperature/Humidity grid (each position shows resulting biome):"
		<< std::endl;
	std::cout << "Humidity →" << std::endl;

	for (int temp = 0; temp < 3; ++temp) {
		if (temp == 1) {
			std::cout << "T ";
		} else {
			std::cout << "e ";
		}

		for (int humid = 0; humid < 3; ++humid) {
			double temperature
				= static_cast<double>(temp) / 2.0; // 0.0, 0.5, 1.0
			double humidity = static_cast<double>(humid) / 2.0;

			BiomeType biome = determine_biome(temperature, humidity);
			std::cout << get_biome_char(biome) << " ";
		}

		if (temp == 1) {
			std::cout << " ← Temperature";
		}
		std::cout << std::endl;

		if (temp == 0) {
			std::cout << "m ";
		} else if (temp == 2) {
			std::cout << "p ";
		} else {
			std::cout << "  ";
		}
	}

	std::cout << std::endl;
	std::cout << "Where: Cold(top) → Hot(bottom), Dry(left) → Wet(right)"
			  << std::endl;

	return 0;
}
