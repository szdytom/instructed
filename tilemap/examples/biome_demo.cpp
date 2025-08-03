#include "bmp.h"
#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <chrono>
#include <cstdlib>
#include <format>
#include <print>
#include <string>

// Get BMP color for different tile types, considering surface tiles
BmpColors::Color get_tile_color(const istd::Tile &tile) {
	// Oil surface tile overrides base color
	if (tile.surface == istd::SurfaceTileType::Oil) {
		return BmpColors::OIL;
	}

	// Otherwise use base tile color
	switch (tile.base) {
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
	case istd::BaseTileType::Deepwater:
		return BmpColors::DEEPWATER;
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

					auto color = get_tile_color(tile);

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
		std::println(stderr, "Error: Could not save BMP file: {}", filename);
		return;
	}

	std::println("BMP file generated: {}", filename);
	std::println("Image size: {}x{} pixels", image_size, image_size);
	std::println("Tilemap size: {}x{} tiles", total_tiles, total_tiles);
	std::println("Chunks: {}x{}", chunks_per_side, chunks_per_side);
}

// Print statistics about the generated map
void print_statistics(const istd::TileMap &tilemap) {
	int tile_counts[6] = {
		0
	}; // Count for each base tile type (now 6 types including Deepwater)
	int oil_count = 0; // Count oil surface tiles
	const int chunks_per_side = tilemap.get_size();
	const int tiles_per_chunk = istd::Chunk::size;

	for (int chunk_x = 0; chunk_x < chunks_per_side; ++chunk_x) {
		for (int chunk_y = 0; chunk_y < chunks_per_side; ++chunk_y) {
			const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
				for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
					const auto &tile = chunk.tiles[tile_x][tile_y];
					tile_counts[static_cast<int>(tile.base)]++;

					// Count oil surface tiles
					if (tile.surface == istd::SurfaceTileType::Oil) {
						oil_count++;
					}
				}
			}
		}
	}

	const char *tile_names[] = {"Land",  "Mountain", "Sand",
	                            "Water", "Ice",      "Deepwater"};
	int total_tiles = chunks_per_side * chunks_per_side * tiles_per_chunk
		* tiles_per_chunk;

	std::println("\nTile Statistics:");
	std::println("================");
	for (int i = 0; i < 6; ++i) {
		double percentage = (double)tile_counts[i] / total_tiles * 100.0;
		std::println(
			"{:>10}: {:>8} ({:.1f}%)", tile_names[i], tile_counts[i], percentage
		);
	}

	// Print oil statistics
	double oil_percentage = (double)oil_count / total_tiles * 100.0;
	double oil_per_chunk = (double)oil_count
		/ (chunks_per_side * chunks_per_side);
	std::println(
		"{:>10}: {:>8} ({:.1f}%, {:.2f} per chunk)", "Oil", oil_count,
		oil_percentage, oil_per_chunk
	);

	std::println("Total tiles: {}", total_tiles);
}

int main(int argc, char *argv[]) {
	// Parse command line arguments
	if (argc > 4) {
		std::println(
			stderr, "Usage: {} <seed> <output_file.bmp> [chunks_per_side]",
			argv[0]
		);
		std::println(stderr, "  seed           - Random seed for generation");
		std::println(stderr, "  output_file    - Output BMP filename");
		std::println(
			stderr, "  chunks_per_side- Number of chunks per side (default: 4)"
		);
		std::println(stderr, "Example: {} 12345 output.bmp 6", argv[0]);
		return 1;
	}

	istd::Seed seed = istd::Seed::from_string(
		argc >= 2 ? argv[1] : "hello_world"
	);
	std::string output_filename = argc >= 3 ? argv[2] : "output.bmp";
	int chunks_per_side = 8; // Default value

	// Parse optional chunks_per_side parameter
	if (argc == 4) {
		chunks_per_side = std::atoi(argv[3]);
		if (chunks_per_side <= 0) {
			std::println(
				stderr, "Error: chunks_per_side must be a positive integer"
			);
			return 1;
		}
		if (chunks_per_side > 20) {
			std::println(
				stderr,
				"Warning: Large chunk counts may produce very large images"
			);
		}
	}

	// Validate output filename
	if (output_filename.length() < 4
	    || output_filename.substr(output_filename.length() - 4) != ".bmp") {
		std::println(stderr, "Error: Output filename must end with .bmp");
		return 1;
	}

	std::println(
		"Generating {}x{} chunk tilemap with seed: {}, {}", chunks_per_side,
		chunks_per_side, seed.s[0], seed.s[1]
	);

	// Create tilemap with specified size
	istd::TileMap tilemap(chunks_per_side);

	// Configure generation parameters
	istd::GenerationConfig config;
	config.seed = seed;

	// Generate the map
	std::println("Generating terrain...");

	// Start timing
	auto start_time = std::chrono::high_resolution_clock::now();

	istd::map_generate(tilemap, config);

	// End timing and calculate duration
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time
	);

	// Convert to appropriate units for display
	if (duration.count() < 1000) {
		std::println(
			"Map generation completed in {} microseconds", duration.count()
		);
	} else if (duration.count() < 1000000) {
		double ms = duration.count() / 1000.0;
		std::println("Map generation completed in {:.2f} milliseconds", ms);
	} else {
		double seconds = duration.count() / 1000000.0;
		std::println("Map generation completed in {:.3f} seconds", seconds);
	}

	// Generate BMP output
	std::println("Creating BMP visualization...");
	generate_bmp(tilemap, output_filename);

	// Print statistics
	print_statistics(tilemap);

	return 0;
}
