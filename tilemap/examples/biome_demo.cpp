#include "bmp.h"
#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

// Get BMP color for different base tile types
BmpColors::Color get_tile_color(istd::BaseTileType type) {
	switch (type) {
	case istd::BaseTileType::Land:
		return BmpColors::LAND;
	case istd::BaseTileType::Mountain:
		return BmpColors::MOUNTAIN;
	case istd::BaseTileType::Sand:
		return BmpColors::SAND;
	case istd::BaseTileType::Water:
		return BmpColors::WATER;
	case istd::BaseTileType::Ice:
		return BmpColors::ICE;
	default:
		return BmpColors::Color(128, 128, 128); // Gray for unknown types
	}
}

// Generate BMP file from tilemap
void generate_bmp(const istd::TileMap &tilemap, const std::string &filename) {
	const int chunks_per_side = tilemap.get_size();
	const int tiles_per_chunk = istd::Chunk::size;
	const int total_tiles = chunks_per_side * tiles_per_chunk;
	const int tile_size = 2; // Size of each tile in pixels
	const int image_size = total_tiles * tile_size;

	BmpWriter bmp(image_size, image_size);

	// Generate tiles
	for (int chunk_y = 0; chunk_y < chunks_per_side; ++chunk_y) {
		for (int chunk_x = 0; chunk_x < chunks_per_side; ++chunk_x) {
			const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
				for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
					const auto &tile = chunk.tiles[tile_x][tile_y];

					int global_x = chunk_x * tiles_per_chunk + tile_x;
					int global_y = chunk_y * tiles_per_chunk + tile_y;

					auto color = get_tile_color(tile.base);

					// Draw a tile_size x tile_size block
					for (int dy = 0; dy < tile_size; ++dy) {
						for (int dx = 0; dx < tile_size; ++dx) {
							int pixel_x = global_x * tile_size + dx;
							int pixel_y = global_y * tile_size + dy;
							bmp.set_pixel(
								pixel_x, pixel_y, color.r, color.g, color.b
							);
						}
					}
				}
			}
		}
	}

	// Add chunk boundary lines (optional - makes file larger)
	/*
	for (int i = 0; i <= chunks_per_side; ++i) {
	    int pos = i * tiles_per_chunk * tile_size;
	    // Vertical lines
	    for (int y = 0; y < image_size; ++y) {
	        if (pos < image_size) {
	            bmp.set_pixel(pos, y, 0, 0, 0); // Black
	        }
	    }
	    // Horizontal lines
	    for (int x = 0; x < image_size; ++x) {
	        if (pos < image_size) {
	            bmp.set_pixel(x, pos, 0, 0, 0); // Black
	        }
	    }
	}
	*/

	if (!bmp.save(filename)) {
		std::cerr << "Error: Could not save BMP file: " << filename
				  << std::endl;
		return;
	}

	std::cout << "BMP file generated: " << filename << std::endl;
	std::cout << "Image size: " << image_size << "x" << image_size << " pixels"
			  << std::endl;
	std::cout << "Tilemap size: " << total_tiles << "x" << total_tiles
			  << " tiles" << std::endl;
	std::cout << "Chunks: " << chunks_per_side << "x" << chunks_per_side
			  << std::endl;
}

// Print statistics about the generated map
void print_statistics(const istd::TileMap &tilemap) {
	int tile_counts[5] = {0}; // Count for each base tile type
	const int chunks_per_side = tilemap.get_size();
	const int tiles_per_chunk = istd::Chunk::size;

	for (int chunk_y = 0; chunk_y < chunks_per_side; ++chunk_y) {
		for (int chunk_x = 0; chunk_x < chunks_per_side; ++chunk_x) {
			const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
				for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
					const auto &tile = chunk.tiles[tile_x][tile_y];
					tile_counts[static_cast<int>(tile.base)]++;
				}
			}
		}
	}

	const char *tile_names[] = {"Land", "Mountain", "Sand", "Water", "Ice"};
	int total_tiles
		= chunks_per_side * chunks_per_side * tiles_per_chunk * tiles_per_chunk;

	std::cout << "\nTile Statistics:\n";
	std::cout << "================\n";
	for (int i = 0; i < 5; ++i) {
		double percentage = (double)tile_counts[i] / total_tiles * 100.0;
		std::cout << std::setw(10) << tile_names[i] << ": " << std::setw(8)
				  << tile_counts[i] << " (" << std::fixed
				  << std::setprecision(1) << percentage << "%)\n";
	}
	std::cout << "Total tiles: " << total_tiles << std::endl;
}

int main(int argc, char *argv[]) {
	// Parse command line arguments
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <seed> <output_file.bmp>\n";
		std::cerr << "Example: " << argv[0] << " 12345 output.bmp\n";
		return 1;
	}

	std::uint64_t seed = std::strtoull(argv[1], nullptr, 10);
	std::string output_filename = argv[2];

	// Validate output filename
	if (output_filename.length() < 4
	    || output_filename.substr(output_filename.length() - 4) != ".bmp") {
		std::cerr << "Error: Output filename must end with .bmp\n";
		return 1;
	}

	std::cout << "Generating 4x4 chunk tilemap with seed: " << seed
			  << std::endl;

	// Create 4x4 chunk tilemap
	istd::TileMap tilemap(4);

	// Configure generation parameters
	istd::GenerationConfig config;
	config.seed = seed;
	config.temperature_scale = 0.005;
	config.humidity_scale = 0.007;
	config.base_scale = 0.08;

	// Generate the map
	std::cout << "Generating terrain..." << std::endl;
	istd::map_generate(tilemap, config);

	// Generate BMP output
	std::cout << "Creating BMP visualization..." << std::endl;
	generate_bmp(tilemap, output_filename);

	// Print statistics
	print_statistics(tilemap);

	return 0;
}