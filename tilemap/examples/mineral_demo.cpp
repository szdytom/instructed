#include "bmp.h"
#include "tilemap/generation.h"
#include "tilemap/tilemap.h"
#include <iostream>

using namespace istd;

int main() {
	constexpr std::uint8_t map_size = 8; // 8x8 chunks
	TileMap tilemap(map_size);

	// Create generation config with adjusted mineral parameters
	GenerationConfig config;
	config.seed = Seed::from_string("mineral_demo_seed");

	// Increase mineral density for demo
	config.hematite_density = 102;       // ~0.4 per chunk
	config.titanomagnetite_density = 76; // ~0.3 per chunk
	config.gibbsite_density = 51;        // ~0.2 per chunk

	// Smaller clusters for better visibility
	config.mineral_cluster_min_size = 1;
	config.mineral_cluster_max_size = 4;

	// Generate the terrain
	map_generate(tilemap, config);

	// Create BMP to visualize the mineral distribution
	constexpr std::uint32_t tile_size = 4; // Each tile is 4x4 pixels
	std::uint32_t image_size = map_size * Chunk::size * tile_size;

	BmpWriter bmp(image_size, image_size);

	// Define colors for different tile types
	auto get_tile_color = [](const Tile &tile)
		-> std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> {
		// Override with mineral colors if present first
		switch (tile.surface) {
		case SurfaceTileType::Oil:
			return {0, 0, 0};     // Black
		case SurfaceTileType::Hematite:
			return {255, 0, 0};   // Red
		case SurfaceTileType::Titanomagnetite:
			return {128, 0, 128}; // Purple
		case SurfaceTileType::Gibbsite:
			return {255, 255, 0}; // Yellow
		case SurfaceTileType::Empty:
		default:
			break;                // Fall through to base terrain colors
		}

		// Base terrain colors
		switch (tile.base) {
		case BaseTileType::Land:
			return {0, 128, 0};     // Green
		case BaseTileType::Mountain:
			return {139, 69, 19};   // Brown
		case BaseTileType::Sand:
			return {238, 203, 173}; // Beige
		case BaseTileType::Water:
			return {0, 0, 255};     // Blue
		case BaseTileType::Ice:
			return {173, 216, 230}; // Light Blue
		case BaseTileType::Deepwater:
			return {0, 0, 139};     // Dark Blue
		default:
			return {128, 128, 128}; // Gray
		}
	};

	// Fill the BMP with tile data
	for (std::uint32_t y = 0; y < image_size; ++y) {
		for (std::uint32_t x = 0; x < image_size; ++x) {
			// Calculate which tile this pixel belongs to
			std::uint32_t tile_x = x / tile_size;
			std::uint32_t tile_y = y / tile_size;

			TilePos pos = TilePos::from_global(tile_x, tile_y);
			const Tile &tile = tilemap.get_tile(pos);

			auto [r, g, b] = get_tile_color(tile);
			bmp.set_pixel(x, y, r, g, b);
		}
	}

	// Save the BMP
	bmp.save("mineral_demo.bmp");

	// Print statistics
	std::uint32_t hematite_count = 0;
	std::uint32_t titanomagnetite_count = 0;
	std::uint32_t gibbsite_count = 0;
	std::uint32_t mountain_edge_count = 0;
	std::uint32_t total_mountain_count = 0;

	std::uint32_t total_tiles = map_size * Chunk::size * map_size * Chunk::size;

	for (std::uint32_t y = 0; y < map_size * Chunk::size; ++y) {
		for (std::uint32_t x = 0; x < map_size * Chunk::size; ++x) {
			TilePos pos = TilePos::from_global(x, y);
			const Tile &tile = tilemap.get_tile(pos);

			if (tile.base == BaseTileType::Mountain) {
				total_mountain_count++;

				// Check if it's a mountain edge
				auto neighbors = tilemap.get_neighbors(pos);
				bool is_edge = false;
				for (const auto neighbor_pos : neighbors) {
					const Tile &neighbor_tile = tilemap.get_tile(neighbor_pos);
					if (neighbor_tile.base != BaseTileType::Mountain) {
						is_edge = true;
						break;
					}
				}
				if (is_edge) {
					mountain_edge_count++;
				}
			}

			switch (tile.surface) {
			case SurfaceTileType::Hematite:
				hematite_count++;
				break;
			case SurfaceTileType::Titanomagnetite:
				titanomagnetite_count++;
				break;
			case SurfaceTileType::Gibbsite:
				gibbsite_count++;
				break;
			default:
				break;
			}
		}
	}

	std::cout << "Mineral Generation Demo Results:\n";
	std::cout << "================================\n";
	std::cout << "Total tiles: " << total_tiles << "\n";
	std::cout << "Mountain tiles: " << total_mountain_count << " ("
			  << (100.0 * total_mountain_count / total_tiles) << "%)\n";
	std::cout << "Mountain edge tiles: " << mountain_edge_count << " ("
			  << (100.0 * mountain_edge_count / total_mountain_count)
			  << "% of mountains)\n";
	std::cout << "\nMineral Distribution:\n";
	std::cout << "Hematite tiles: " << hematite_count << "\n";
	std::cout << "Titanomagnetite tiles: " << titanomagnetite_count << "\n";
	std::cout << "Gibbsite tiles: " << gibbsite_count << "\n";
	std::cout << "Total mineral tiles: "
			  << (hematite_count + titanomagnetite_count + gibbsite_count)
			  << "\n";

	if (mountain_edge_count > 0) {
		double mineral_coverage = 100.0
			* (hematite_count + titanomagnetite_count + gibbsite_count)
			/ mountain_edge_count;
		std::cout << "Mineral coverage on mountain edges: " << mineral_coverage
				  << "%\n";
	}

	std::cout << "\nGenerated mineral_demo.bmp with visualization\n";
	std::cout
		<< "Colors: Red=Hematite, Purple=Titanomagnetite, Yellow=Gibbsite\n";
	std::cout << "        Brown=Mountain, Green=Land, Blue=Water, etc.\n";

	return 0;
}
