#include "generation.h"
#include "tilemap.h"
#include <iomanip>
#include <iostream>

using namespace istd;

// Function to get character representation of base tile type
char get_base_char(BaseTileType base_type) {
	switch (base_type) {
	case BaseTileType::Land:
		return '.';
	case BaseTileType::Mountain:
		return '^';
	case BaseTileType::Sand:
		return '~';
	case BaseTileType::Water:
		return 'W';
	case BaseTileType::Ice:
		return 'I';
	}
	return '?';
}

// Function to get character representation of surface tile type
char get_surface_char(SurfaceTileType surface_type) {
	switch (surface_type) {
	case SurfaceTileType::Empty:
		return ' ';
	case SurfaceTileType::Wood:
		return 'T';
	case SurfaceTileType::Snow:
		return 'S';
	case SurfaceTileType::Structure:
		return 'H';
	}
	return '?';
}

// Function to get combined display character for a tile
char get_tile_char(const Tile &tile) {
	// Surface features take priority for display
	if (tile.surface != SurfaceTileType::Empty) {
		return get_surface_char(tile.surface);
	}
	return get_base_char(tile.base);
}

int main() {
	std::cout << "=== Dual-Noise Terrain Generation Demo ===" << std::endl;
	std::cout << "This demo shows the new base + surface generation system"
			  << std::endl;
	std::cout << "Legend:" << std::endl;
	std::cout
		<< "  . = Land     ^ = Mountain    ~ = Sand     W = Water    I = Ice"
		<< std::endl;
	std::cout << "  T = Trees    S = Snow        H = Structure" << std::endl;
	std::cout << std::endl;

	// Create a small tilemap for demonstration
	constexpr std::uint8_t map_size = 4; // 4x4 chunks
	TileMap tilemap(map_size);

	// Configure generation with different seeds for variety
	GenerationConfig config;
	config.seed = 12345;
	config.temperature_scale = 0.008; // Slightly larger temperature variation
	config.humidity_scale = 0.006;    // Slightly larger humidity variation

	std::cout << "Generating " << static_cast<int>(map_size) << "x"
			  << static_cast<int>(map_size) << " chunks ("
			  << static_cast<int>(map_size * Chunk::size) << "x"
			  << static_cast<int>(map_size * Chunk::size) << " tiles)..."
			  << std::endl;

	// Generate the map
	map_generate(tilemap, config);

	std::cout << "Generation complete!" << std::endl << std::endl;

	// Display the entire map
	std::cout << "Complete Map:" << std::endl;
	for (std::uint8_t global_y = 0; global_y < map_size * Chunk::size;
	     ++global_y) {
		for (std::uint8_t global_x = 0; global_x < map_size * Chunk::size;
		     ++global_x) {
			// Convert global coordinates to chunk and local coordinates
			std::uint8_t chunk_x = global_x / Chunk::size;
			std::uint8_t chunk_y = global_y / Chunk::size;
			std::uint8_t local_x = global_x % Chunk::size;
			std::uint8_t local_y = global_y % Chunk::size;

			TilePos pos{chunk_x, chunk_y, local_x, local_y};
			Tile tile = tilemap.get_tile(pos);
			std::cout << get_tile_char(tile);
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	// Show detailed analysis for each chunk
	std::cout << "=== Chunk-by-Chunk Analysis ===" << std::endl;
	for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			std::cout << "Chunk (" << static_cast<int>(chunk_x) << ","
					  << static_cast<int>(chunk_y) << "):" << std::endl;

			// Count tile types in this chunk
			int base_counts[5] = {0};    // Land, Mountain, Sand, Water, Ice
			int surface_counts[4] = {0}; // Empty, Wood, Snow, Structure

			for (std::uint8_t local_y = 0; local_y < Chunk::size; ++local_y) {
				for (std::uint8_t local_x = 0; local_x < Chunk::size;
				     ++local_x) {
					TilePos pos{chunk_x, chunk_y, local_x, local_y};
					Tile tile = tilemap.get_tile(pos);

					base_counts[static_cast<int>(tile.base)]++;
					surface_counts[static_cast<int>(tile.surface)]++;
				}
			}

			std::cout << "  Base terrain: Land=" << base_counts[0]
					  << " Mountain=" << base_counts[1]
					  << " Sand=" << base_counts[2]
					  << " Water=" << base_counts[3]
					  << " Ice=" << base_counts[4] << std::endl;

			std::cout << "  Surface features: Empty=" << surface_counts[0]
					  << " Wood=" << surface_counts[1]
					  << " Snow=" << surface_counts[2]
					  << " Structure=" << surface_counts[3] << std::endl;
		}
	}

	std::cout << std::endl;
	std::cout << "=== Sample Detailed Tiles ===" << std::endl;

	// Show some individual tile details
	std::vector<TilePos> sample_positions = {
		{0, 0, 10, 10},
        {1, 1, 30, 30},
        {2, 2, 50, 50},
        {3, 3, 60, 60}
	};

	for (const auto &pos : sample_positions) {
		if (pos.chunk_x < map_size && pos.chunk_y < map_size
		    && pos.local_x < Chunk::size && pos.local_y < Chunk::size) {
			Tile tile = tilemap.get_tile(pos);
			std::cout << "Tile at chunk(" << static_cast<int>(pos.chunk_x)
					  << "," << static_cast<int>(pos.chunk_y) << ") local("
					  << static_cast<int>(pos.local_x) << ","
					  << static_cast<int>(pos.local_y) << "): ";

			std::cout << "Base=" << get_base_char(tile.base)
					  << " Surface=" << get_surface_char(tile.surface)
					  << " Display=" << get_tile_char(tile) << std::endl;
		}
	}

	return 0;
}
