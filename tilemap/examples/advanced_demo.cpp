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

// Function to count tile types in a region
std::map<std::uint8_t, int> count_tiles(
	const istd::TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	std::uint8_t start_x, std::uint8_t start_y, std::uint8_t end_x,
	std::uint8_t end_y
) {
	std::map<std::uint8_t, int> counts;

	for (std::uint8_t y = start_y; y < end_y; ++y) {
		for (std::uint8_t x = start_x; x < end_x; ++x) {
			istd::TilePos pos{chunk_x, chunk_y, x, y};
			istd::Tile tile = tilemap.get_tile(pos);
			counts[tile.type]++;
		}
	}

	return counts;
}

int main() {
	try {
		// Test with a larger map (8x8 chunks)
		std::uint8_t map_size = 8;
		istd::TileMap tilemap(map_size);

		std::cout << "=== Perlin Noise Tilemap Generation Demo ==="
				  << std::endl;
		std::cout << "Generating " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size) << " chunk tilemap..."
				  << std::endl;

		// Configure generation parameters for more interesting terrain
		istd::GenerationConfig config;
		config.seed = 42;
		config.scale = 0.02; // Smaller scale for larger features
		config.octaves = 5;  // More octaves for detail
		config.persistence = 0.5;

		// Better balanced thresholds
		config.water_threshold = 0.3;
		config.sand_threshold = 0.45;
		config.wood_threshold = 0.7;
		config.mountain_threshold = 0.85;

		// Generate the map
		istd::map_generate(tilemap, config);

		std::cout << "Map generated successfully!" << std::endl;
		std::cout
			<< "Legend: ' '=empty, '^'=mountain, 'T'=wood, '.'=sand, '~'=water"
			<< std::endl
			<< std::endl;

		// Display multiple sample areas
		std::cout << "Sample areas from different chunks:" << std::endl;

		// Show 4 corners of the map
		std::vector<std::pair<std::uint8_t, std::uint8_t>> sample_chunks = {
			{0,            0           },
			{map_size - 1, 0           },
			{0,            map_size - 1},
			{map_size - 1, map_size - 1}
		};

		for (const auto &chunk_pos : sample_chunks) {
			std::cout << "Chunk (" << static_cast<int>(chunk_pos.first) << ","
					  << static_cast<int>(chunk_pos.second)
					  << ") - top-left 20x10:" << std::endl;

			const istd::Chunk &chunk
				= tilemap.get_chunk(chunk_pos.first, chunk_pos.second);

			// Display 20x10 area from this chunk
			for (int y = 0; y < 10; ++y) {
				for (int x = 0; x < 20; ++x) {
					std::cout << get_tile_char(chunk.tiles[y][x]);
				}
				std::cout << std::endl;
			}

			// Count tile distribution in this 20x10 area
			auto counts = count_tiles(
				tilemap, chunk_pos.first, chunk_pos.second, 0, 0, 20, 10
			);
			std::cout << "Distribution: ";
			for (const auto &pair : counts) {
				std::cout << get_tile_char(istd::Tile{pair.first}) << ":"
						  << pair.second << " ";
			}
			std::cout << std::endl << std::endl;
		}

		// Overall statistics
		std::cout << "=== Overall Map Statistics ===" << std::endl;
		std::cout << "- Map size: " << static_cast<int>(map_size) << "x"
				  << static_cast<int>(map_size) << " chunks" << std::endl;
		std::cout << "- Chunk size: " << static_cast<int>(istd::Chunk::size)
				  << "x" << static_cast<int>(istd::Chunk::size) << " tiles"
				  << std::endl;
		std::cout << "- Total tiles: "
				  << static_cast<int>(map_size) * static_cast<int>(map_size)
						 * static_cast<int>(istd::Chunk::size)
						 * static_cast<int>(istd::Chunk::size)
				  << std::endl;
		std::cout << "- Total chunks: "
				  << static_cast<int>(map_size) * static_cast<int>(map_size)
				  << std::endl;

		// Test TilePos functionality
		std::cout << std::endl << "=== TilePos Testing ===" << std::endl;
		std::vector<istd::TilePos> test_positions = {
			{0,            0,            0,                     0}, // Top-left corner
			{map_size - 1, map_size - 1, istd::Chunk::size - 1,
		     istd::Chunk::size - 1                               }, // Bottom-right corner
			{map_size / 2, map_size / 2, istd::Chunk::size / 2,
		     istd::Chunk::size / 2                               }, // Center
		};

		for (const auto &pos : test_positions) {
			istd::Tile tile = tilemap.get_tile(pos);
			std::cout << "Tile at chunk(" << static_cast<int>(pos.chunk_x)
					  << "," << static_cast<int>(pos.chunk_y) << ") local("
					  << static_cast<int>(pos.local_x) << ","
					  << static_cast<int>(pos.local_y)
					  << "): " << get_tile_char(tile) << " (type "
					  << static_cast<int>(tile.type) << ")" << std::endl;
		}

		// Test tile modification
		std::cout << std::endl << "=== Tile Modification Test ===" << std::endl;
		istd::TilePos modify_pos{1, 1, 30, 30};
		istd::Tile original_tile = tilemap.get_tile(modify_pos);
		std::cout << "Original tile: " << get_tile_char(original_tile)
				  << std::endl;

		// Change it to mountain
		istd::Tile mountain_tile = istd::Tile::from_name("mountain");
		tilemap.set_tile(modify_pos, mountain_tile);

		istd::Tile modified_tile = tilemap.get_tile(modify_pos);
		std::cout << "Modified tile: " << get_tile_char(modified_tile)
				  << std::endl;

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
