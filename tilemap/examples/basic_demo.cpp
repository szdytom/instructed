#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <iostream>

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

int main() {
	try {
		std::cout << "=== Perlin Noise Tilemap Generator Demo ===" << std::endl;

		// Create a 6x6 chunk tilemap
		std::uint8_t map_size = 6;
		istd::TileMap tilemap(map_size);

		// Configure generation parameters
		istd::GenerationConfig config;
		config.seed = 2024;
		config.scale = 0.03;
		config.octaves = 4;
		config.persistence = 0.6;

		// Generate the map using Perlin noise
		std::cout << "Generating " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size)
				  << " chunk map with Perlin noise..." << std::endl;
		istd::map_generate(tilemap, config);
		std::cout << "Generation complete!" << std::endl << std::endl;

		// Display a sample area
		std::cout << "Sample from center chunk (3,3):" << std::endl;
		std::cout
			<< "Legend: ' '=empty, '^'=mountain, 'T'=wood, '.'=sand, '~'=water"
			<< std::endl;

		const istd::Chunk &center_chunk = tilemap.get_chunk(3, 3);
		for (int y = 20; y < 44; ++y) {
			for (int x = 20; x < 44; ++x) {
				std::cout << get_tile_char(center_chunk.tiles[y][x]);
			}
			std::cout << std::endl;
		}

		// Demonstrate TilePos usage
		std::cout << std::endl << "TilePos demonstration:" << std::endl;
		istd::TilePos test_pos{2, 3, 15, 25};
		istd::Tile original = tilemap.get_tile(test_pos);
		std::cout << "Tile at chunk(2,3) local(15,25): "
				  << get_tile_char(original) << std::endl;

		// Modify the tile
		istd::Tile water = istd::Tile::from_name("water");
		tilemap.set_tile(test_pos, water);
		istd::Tile modified = tilemap.get_tile(test_pos);
		std::cout << "After setting to water: " << get_tile_char(modified)
				  << std::endl;

		std::cout << std::endl << "Map Statistics:" << std::endl;
		std::cout << "- Total chunks: " << static_cast<int>(map_size * map_size)
				  << std::endl;
		std::cout << "- Total tiles: "
				  << static_cast<int>(map_size * map_size * 64 * 64)
				  << std::endl;
		std::cout << "- Memory usage: ~"
				  << static_cast<int>(map_size * map_size * 64 * 64) << " bytes"
				  << std::endl;

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
