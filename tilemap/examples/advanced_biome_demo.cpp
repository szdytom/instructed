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

// Function to create biome map visualization
void show_biome_map(
	std::uint8_t map_size, const istd::GenerationConfig &config
) {
	std::cout << "=== Biome Map Visualization ===" << std::endl;

	// Create noise generators for climate
	istd::PerlinNoise temp_noise(config.seed + 1000);
	istd::PerlinNoise humidity_noise(config.seed + 2000);

	// Generate biome map for visualization
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t sub_y = 0; sub_y < 4; ++sub_y) {
			for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
				for (std::uint8_t sub_x = 0; sub_x < 4; ++sub_x) {
					// Calculate global position for this sub-chunk's center
					double global_x = static_cast<double>(
						chunk_x * istd::Chunk::size + sub_x * 16 + 8
					);
					double global_y = static_cast<double>(
						chunk_y * istd::Chunk::size + sub_y * 16 + 8
					);

					// Get climate values
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
					std::cout << get_biome_char(biome);
				}
			}
			std::cout << std::endl;
		}
	}

	std::cout << std::endl << "Biome Legend:" << std::endl;
	std::cout << "D=Desert, S=Savanna, R=TropicalRainforest, G=Grassland"
			  << std::endl;
	std::cout << "F=DeciduousForest, M=TemperateRainforest, U=Tundra, A=Taiga, "
	             "C=ColdRainforest"
			  << std::endl;
	std::cout << std::endl;
}

// Function to analyze terrain distribution in different biomes
void analyze_biome_terrain(
	const istd::TileMap &tilemap, std::uint8_t map_size,
	const istd::GenerationConfig &config
) {
	std::cout << "=== Biome Terrain Analysis ===" << std::endl;

	// Count tiles for each biome
	std::map<istd::BiomeType, std::map<std::uint8_t, int>> biome_tile_counts;

	// Create noise generators for climate
	istd::PerlinNoise temp_noise(config.seed + 1000);
	istd::PerlinNoise humidity_noise(config.seed + 2000);

	// Sample terrain from different sub-chunks
	for (std::uint8_t chunk_y = 0; chunk_y < map_size;
	     chunk_y += 2) {       // Sample every other chunk
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; chunk_x += 2) {
			for (std::uint8_t sub_y = 0; sub_y < 4;
			     sub_y += 2) { // Sample every other sub-chunk
				for (std::uint8_t sub_x = 0; sub_x < 4; sub_x += 2) {
					// Determine biome for this sub-chunk
					double global_x = static_cast<double>(
						chunk_x * istd::Chunk::size + sub_x * 16 + 8
					);
					double global_y = static_cast<double>(
						chunk_y * istd::Chunk::size + sub_y * 16 + 8
					);

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

					// Sample 8x8 area from center of sub-chunk
					auto [start_x, start_y] = istd::subchunk_to_tile_start(
						istd::SubChunkPos(sub_x, sub_y)
					);
					for (std::uint8_t y = start_y + 4; y < start_y + 12; ++y) {
						for (std::uint8_t x = start_x + 4; x < start_x + 12;
						     ++x) {
							istd::TilePos pos{chunk_x, chunk_y, x, y};
							istd::Tile tile = tilemap.get_tile(pos);
							biome_tile_counts[biome][tile.type]++;
						}
					}
				}
			}
		}
	}

	// Display results
	for (const auto &[biome, tile_counts] : biome_tile_counts) {
		const auto &props = istd::get_biome_properties(biome);
		std::cout << props.name << ":" << std::endl;

		int total = 0;
		for (const auto &[tile_type, count] : tile_counts) {
			total += count;
		}

		for (const auto &[tile_type, count] : tile_counts) {
			double percentage = (static_cast<double>(count) / total) * 100.0;
			std::cout << "  " << get_tile_char(istd::Tile{tile_type}) << ": "
					  << std::fixed << std::setprecision(1) << percentage << "%"
					  << std::endl;
		}
		std::cout << std::endl;
	}
}

int main() {
	try {
		std::cout << "=== Advanced Biome System Demo ===" << std::endl;

		// Create a larger map to show more biome diversity
		std::uint8_t map_size = 8;
		istd::TileMap tilemap(map_size);

		// Configure generation for more diverse biomes
		istd::GenerationConfig config;
		config.seed = 1337;
		config.temperature_scale = 0.01; // More variation in temperature
		config.humidity_scale = 0.012;   // More variation in humidity

		std::cout << "Generating " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size)
				  << " chunk map with diverse biomes..." << std::endl;

		// Show biome distribution before generating terrain
		show_biome_map(map_size, config);

		// Generate the terrain
		istd::map_generate(tilemap, config);
		std::cout << "Terrain generation complete!" << std::endl << std::endl;

		// Analyze terrain distribution in different biomes
		analyze_biome_terrain(tilemap, map_size, config);

		// Show terrain samples from different biomes
		std::cout << "=== Terrain Samples by Biome ===" << std::endl;
		std::cout
			<< "Legend: ' '=empty, '^'=mountain, 'T'=wood, '.'=sand, '~'=water"
			<< std::endl
			<< std::endl;

		// Sample specific locations with known biomes
		struct BiomeSample {
			std::uint8_t chunk_x, chunk_y, sub_x, sub_y;
			const char *expected_biome;
		};

		std::vector<BiomeSample> samples = {
			{1, 1, 1, 1, "Top-left region"    },
			{6, 1, 2, 1, "Top-right region"   },
			{1, 6, 1, 2, "Bottom-left region" },
			{6, 6, 2, 2, "Bottom-right region"},
			{3, 3, 1, 1, "Center region"      }
		};

		// Create noise generators for biome determination
		istd::PerlinNoise temp_noise(config.seed + 1000);
		istd::PerlinNoise humidity_noise(config.seed + 2000);

		for (const auto &sample : samples) {
			// Determine biome for this sample
			double global_x = static_cast<double>(
				sample.chunk_x * istd::Chunk::size + sample.sub_x * 16 + 8
			);
			double global_y = static_cast<double>(
				sample.chunk_y * istd::Chunk::size + sample.sub_y * 16 + 8
			);

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
			const auto &props = istd::get_biome_properties(biome);

			std::cout << sample.expected_biome << " - " << props.name
					  << " (T:" << std::fixed << std::setprecision(2)
					  << temperature << " H:" << humidity << "):" << std::endl;

			// Show 10x6 terrain sample
			auto [start_x, start_y] = istd::subchunk_to_tile_start(
				istd::SubChunkPos(sample.sub_x, sample.sub_y)
			);
			for (std::uint8_t y = start_y + 3; y < start_y + 9; ++y) {
				for (std::uint8_t x = start_x + 3; x < start_x + 13; ++x) {
					istd::TilePos pos{sample.chunk_x, sample.chunk_y, x, y};
					istd::Tile tile = tilemap.get_tile(pos);
					std::cout << get_tile_char(tile);
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		// Show statistics
		std::cout << "=== Map Statistics ===" << std::endl;
		std::cout << "- Map size: " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size) << " chunks" << std::endl;
		std::cout << "- Total sub-chunks: "
				  << static_cast<int>(map_size) * static_cast<int>(map_size)
						 * 16
				  << std::endl;
		std::cout << "- Climate scales: T=" << config.temperature_scale
				  << ", H=" << config.humidity_scale << std::endl;
		std::cout << "- Each sub-chunk represents a 16x16 tile area with "
		             "consistent biome"
				  << std::endl;

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
