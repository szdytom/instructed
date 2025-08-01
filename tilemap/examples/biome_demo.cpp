#include "biome.h"
#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <iomanip>
#include <iostream>
#include <map>

// Helper function to get tile character for display
char get_tile_char(const istd::Tile &tile) {
	switch (tile.type) {
	case 0:
		return ' '; // empty
	case 1:
		return '^'; // mountain
	case 2:
		return 'T'; // wood
	case 3:
		return '.'; // sand
	case 4:
		return '~'; // water
	default:
		return '?';
	}
}

// Helper function to get biome character for display
char get_biome_char(istd::BiomeType biome) {
	switch (biome) {
	case istd::BiomeType::Desert:
		return 'D';
	case istd::BiomeType::Savanna:
		return 'S';
	case istd::BiomeType::TropicalRainforest:
		return 'R';
	case istd::BiomeType::Grassland:
		return 'G';
	case istd::BiomeType::DeciduousForest:
		return 'F';
	case istd::BiomeType::TemperateRainforest:
		return 'M';
	case istd::BiomeType::Tundra:
		return 'U';
	case istd::BiomeType::Taiga:
		return 'A';
	case istd::BiomeType::ColdRainforest:
		return 'C';
	default:
		return '?';
	}
}

// Function to analyze biome distribution in a chunk
void analyze_chunk_biomes(
	const istd::TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y
) {
	std::cout << "Analyzing chunk (" << static_cast<int>(chunk_x) << ","
			  << static_cast<int>(chunk_y)
			  << ") biome distribution:" << std::endl;

	// Create a temporary generator to get climate data
	istd::GenerationConfig config;
	config.seed = 12345;
	istd::TerrainGenerator temp_generator(config);

	// Count biomes in each sub-chunk
	std::map<istd::BiomeType, int> biome_counts;

	for (std::uint8_t sub_y = 0; sub_y < 4; ++sub_y) {
		for (std::uint8_t sub_x = 0; sub_x < 4; ++sub_x) {
			// Calculate global position for this sub-chunk's center
			double global_x = static_cast<double>(
				chunk_x * istd::Chunk::size + sub_x * 16 + 8
			);
			double global_y = static_cast<double>(
				chunk_y * istd::Chunk::size + sub_y * 16 + 8
			);

			// Get climate values (we need to recreate this logic since the
			// generator's method is private)
			istd::PerlinNoise temp_noise(config.seed + 1000);
			istd::PerlinNoise humidity_noise(config.seed + 2000);

			double temperature = temp_noise.octave_noise(
				global_x * config.temperature_scale,
				global_y * config.temperature_scale, 3, 0.5
			);

			double humidity = humidity_noise.octave_noise(
				global_x * config.humidity_scale,
				global_y * config.humidity_scale, 3, 0.5
			);

			istd::BiomeType biome
				= istd::determine_biome(temperature, humidity);
			biome_counts[biome]++;

			std::cout << get_biome_char(biome);
			if (sub_x == 3) {
				std::cout << std::endl;
			}
		}
	}

	std::cout << std::endl << "Biome legend:" << std::endl;
	std::cout << "D=Desert, S=Savanna, R=TropicalRainforest, G=Grassland"
			  << std::endl;
	std::cout << "F=DeciduousForest, M=TemperateRainforest, U=Tundra, A=Taiga, "
	             "C=ColdRainforest"
			  << std::endl;

	std::cout << std::endl << "Sub-chunk counts:" << std::endl;
	for (const auto &[biome, count] : biome_counts) {
		const auto &props = istd::get_biome_properties(biome);
		std::cout << "- " << props.name << ": " << count << " sub-chunks"
				  << std::endl;
	}
	std::cout << std::endl;
}

// Function to show terrain sample from a specific sub-chunk
void show_subchunk_terrain(
	const istd::TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	std::uint8_t sub_x, std::uint8_t sub_y
) {
	std::cout << "Terrain sample from chunk(" << static_cast<int>(chunk_x)
			  << "," << static_cast<int>(chunk_y) << ") sub-chunk("
			  << static_cast<int>(sub_x) << "," << static_cast<int>(sub_y)
			  << "):" << std::endl;

	auto [start_x, start_y]
		= istd::subchunk_to_tile_start(istd::SubChunkPos(sub_x, sub_y));

	// Show 8x8 sample from the center of the sub-chunk
	std::uint8_t sample_start_x = start_x + 4;
	std::uint8_t sample_start_y = start_y + 4;

	for (std::uint8_t y = sample_start_y; y < sample_start_y + 8; ++y) {
		for (std::uint8_t x = sample_start_x; x < sample_start_x + 8; ++x) {
			istd::TilePos pos{chunk_x, chunk_y, x, y};
			istd::Tile tile = tilemap.get_tile(pos);
			std::cout << get_tile_char(tile);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main() {
	try {
		std::cout << "=== Biome-Based Terrain Generation Demo ===" << std::endl;

		// Create a 6x6 chunk tilemap
		std::uint8_t map_size = 6;
		istd::TileMap tilemap(map_size);

		// Configure generation with biome system
		istd::GenerationConfig config;
		config.seed = 42;
		config.temperature_scale = 0.003; // Larger climate features
		config.humidity_scale = 0.004;

		std::cout << "Generating " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size)
				  << " chunk map with biome system..." << std::endl;

		// Generate the map using the new biome-based system
		istd::map_generate(tilemap, config);

		std::cout << "Generation complete!" << std::endl << std::endl;

		// Analyze biome distribution in a few chunks
		std::cout << "=== Biome Analysis ===" << std::endl;
		analyze_chunk_biomes(tilemap, 1, 1);
		analyze_chunk_biomes(tilemap, 4, 4);

		// Show terrain samples from different sub-chunks
		std::cout << "=== Terrain Samples ===" << std::endl;
		std::cout
			<< "Legend: ' '=empty, '^'=mountain, 'T'=wood, '.'=sand, '~'=water"
			<< std::endl
			<< std::endl;

		show_subchunk_terrain(tilemap, 1, 1, 0, 0); // Top-left sub-chunk
		show_subchunk_terrain(tilemap, 1, 1, 3, 3); // Bottom-right sub-chunk
		show_subchunk_terrain(tilemap, 4, 4, 1, 2); // Different chunk

		// Show overall statistics
		std::cout << "=== Map Statistics ===" << std::endl;
		std::cout << "- Map size: " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size) << " chunks" << std::endl;
		std::cout << "- Sub-chunks per chunk: 4x4 (16 total)" << std::endl;
		std::cout << "- Tiles per sub-chunk: 16x16 (256 total)" << std::endl;
		std::cout << "- Total sub-chunks: "
				  << static_cast<int>(map_size) * static_cast<int>(map_size)
						 * 16
				  << std::endl;
		std::cout << "- Total tiles: "
				  << static_cast<int>(map_size) * static_cast<int>(map_size)
						 * static_cast<int>(istd::Chunk::size)
						 * static_cast<int>(istd::Chunk::size)
				  << std::endl;

		// Test coordinate conversion functions
		std::cout << std::endl << "=== Coordinate System Test ===" << std::endl;

		// Test tile to sub-chunk conversion
		istd::SubChunkPos sub_pos = istd::tile_to_subchunk(25, 40);
		std::cout << "Tile (25,40) is in sub-chunk ("
				  << static_cast<int>(sub_pos.sub_x) << ","
				  << static_cast<int>(sub_pos.sub_y) << ")" << std::endl;

		// Test sub-chunk to tile conversion
		auto [tile_start_x, tile_start_y]
			= istd::subchunk_to_tile_start(istd::SubChunkPos(2, 1));
		std::cout << "Sub-chunk (2,1) starts at tile ("
				  << static_cast<int>(tile_start_x) << ","
				  << static_cast<int>(tile_start_y) << ")" << std::endl;

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
