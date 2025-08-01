#include "generation.h"
#include "tile.h"
#include "tilemap.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

// Color mapping for different base tile types
const char *get_tile_color(istd::BaseTileType type) {
	switch (type) {
	case istd::BaseTileType::Land:
		return "#90EE90"; // Light green
	case istd::BaseTileType::Mountain:
		return "#8B4513"; // Saddle brown
	case istd::BaseTileType::Sand:
		return "#F4A460"; // Sandy brown
	case istd::BaseTileType::Water:
		return "#1E90FF"; // Dodger blue
	case istd::BaseTileType::Ice:
		return "#B0E0E6"; // Powder blue
	default:
		return "#808080"; // Gray for unknown types
	}
}

// Generate SVG file from tilemap
void generate_svg(const istd::TileMap &tilemap, const std::string &filename) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open output file: " << filename
				  << std::endl;
		return;
	}

	const int chunks_per_side = tilemap.get_size();
	const int tiles_per_chunk = istd::Chunk::size;
	const int total_tiles = chunks_per_side * tiles_per_chunk;
	const int tile_size = 2; // Size of each tile in SVG pixels
	const int svg_size = total_tiles * tile_size;

	// SVG header
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	file << "<svg width=\"" << svg_size << "\" height=\"" << svg_size
		 << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	file << "<title>Tilemap Visualization</title>\n";

	// Generate tiles
	for (int chunk_y = 0; chunk_y < chunks_per_side; ++chunk_y) {
		for (int chunk_x = 0; chunk_x < chunks_per_side; ++chunk_x) {
			const auto &chunk = tilemap.get_chunk(chunk_x, chunk_y);

			for (int tile_y = 0; tile_y < tiles_per_chunk; ++tile_y) {
				for (int tile_x = 0; tile_x < tiles_per_chunk; ++tile_x) {
					const auto &tile = chunk.tiles[tile_x][tile_y];

					int global_x = chunk_x * tiles_per_chunk + tile_x;
					int global_y = chunk_y * tiles_per_chunk + tile_y;

					int svg_x = global_x * tile_size;
					int svg_y = global_y * tile_size;

					const char *color = get_tile_color(tile.base);

					file << "<rect x=\"" << svg_x << "\" y=\"" << svg_y
						 << "\" width=\"" << tile_size << "\" height=\""
						 << tile_size << "\" fill=\"" << color << "\"/>\n";
				}
			}
		}
	}

	// Add grid lines for chunk boundaries
	file << "<!-- Chunk boundaries -->\n";
	for (int i = 0; i <= chunks_per_side; ++i) {
		int pos = i * tiles_per_chunk * tile_size;
		// Vertical lines
		file << "<line x1=\"" << pos << "\" y1=\"0\" x2=\"" << pos << "\" y2=\""
			 << svg_size << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
		// Horizontal lines
		file << "<line x1=\"0\" y1=\"" << pos << "\" x2=\"" << svg_size
			 << "\" y2=\"" << pos
			 << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
	}

	// Legend
	file << "<!-- Legend -->\n";
	file << "<g transform=\"translate(10, 10)\">\n";
	file << "<rect x=\"0\" y=\"0\" width=\"200\" height=\"140\" fill=\"white\" "
	        "stroke=\"black\" stroke-width=\"1\" opacity=\"0.9\"/>\n";
	file << "<text x=\"10\" y=\"20\" font-family=\"Arial\" font-size=\"14\" "
	        "font-weight=\"bold\">Legend</text>\n";

	const std::pair<istd::BaseTileType, const char *> legend_items[] = {
		{istd::BaseTileType::Land,     "Land"    },
		{istd::BaseTileType::Mountain, "Mountain"},
		{istd::BaseTileType::Sand,     "Sand"    },
		{istd::BaseTileType::Water,    "Water"   },
		{istd::BaseTileType::Ice,      "Ice"     }
	};

	for (int i = 0; i < 5; ++i) {
		int y_pos = 40 + i * 20;
		file << "<rect x=\"10\" y=\"" << (y_pos - 10)
			 << "\" width=\"15\" height=\"15\" fill=\""
			 << get_tile_color(legend_items[i].first)
			 << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
		file << "<text x=\"30\" y=\"" << y_pos
			 << "\" font-family=\"Arial\" font-size=\"12\">"
			 << legend_items[i].second << "</text>\n";
	}
	file << "</g>\n";

	file << "</svg>\n";
	file.close();

	std::cout << "SVG file generated: " << filename << std::endl;
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
		std::cerr << "Usage: " << argv[0] << " <seed> <output_file.svg>\n";
		std::cerr << "Example: " << argv[0] << " 12345 output.svg\n";
		return 1;
	}

	std::uint64_t seed = std::strtoull(argv[1], nullptr, 10);
	std::string output_filename = argv[2];

	// Validate output filename
	if (output_filename.length() < 4
	    || output_filename.substr(output_filename.length() - 4) != ".svg") {
		std::cerr << "Error: Output filename must end with .svg\n";
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

	// Generate SVG output
	std::cout << "Creating SVG visualization..." << std::endl;
	generate_svg(tilemap, output_filename);

	// Print statistics
	print_statistics(tilemap);

	return 0;
}