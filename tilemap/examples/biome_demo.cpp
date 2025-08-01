#include "bmp.h"
#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <cstdlib>
#include <format>
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
			for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
				for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
					istd::TilePos pos(chunk_x, chunk_y, tile_x, tile_y);
					const auto &tile = tilemap.get_tile(pos);

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

	for (int chunk_x = 0; chunk_x < chunks_per_side; ++chunk_x) {
		for (int chunk_y = 0; chunk_y < chunks_per_side; ++chunk_y) {
			const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
				for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
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
	if (argc > 4) {
		std::cerr << "Usage: " << argv[0]
				  << " <seed> <output_file.bmp> [chunks_per_side]\n";
		std::cerr << "  seed           - Random seed for generation\n";
		std::cerr << "  output_file    - Output BMP filename\n";
		std::cerr
			<< "  chunks_per_side- Number of chunks per side (default: 4)\n";
		std::cerr << "Example: " << argv[0] << " 12345 output.bmp 6\n";
		return 1;
	}

	istd::Seed seed
		= istd::Seed::from_string(argc >= 2 ? argv[1] : "hello_world");
	std::string output_filename = argc >= 3 ? argv[2] : "output.bmp";
	int chunks_per_side = 4; // Default value

	// Parse optional chunks_per_side parameter
	if (argc == 4) {
		chunks_per_side = std::atoi(argv[3]);
		if (chunks_per_side <= 0) {
			std::cerr << "Error: chunks_per_side must be a positive integer\n";
			return 1;
		}
		if (chunks_per_side > 20) {
			std::cerr << "Warning: Large chunk counts may produce very large "
						 "images\n";
		}
	}

	// Validate output filename
	if (output_filename.length() < 4
	    || output_filename.substr(output_filename.length() - 4) != ".bmp") {
		std::cerr << "Error: Output filename must end with .bmp\n";
		return 1;
	}

	std::cout << "Generating " << chunks_per_side << "x" << chunks_per_side
			  << " chunk tilemap with seed: " << seed.s[0] << ", " << seed.s[1]
			  << std::endl;

	// Create tilemap with specified size
	istd::TileMap tilemap(chunks_per_side);

	// Configure generation parameters
	istd::GenerationConfig config;
	config.seed = seed;

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
