#include "noise.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Convert a noise value [0,1] to a grayscale color
std::string noise_to_grayscale(double noise_value) {
	// Clamp to [0,1] range
	noise_value = std::max(0.0, std::min(1.0, noise_value));

	// Convert to 0-255 range
	int gray = static_cast<int>(noise_value * 255);

	// Convert to hex color
	std::ostringstream oss;
	oss << "#" << std::hex << std::setfill('0') << std::setw(2) << gray
		<< std::setw(2) << gray << std::setw(2) << gray;
	return oss.str();
}

// Generate comparison SVG showing both raw and uniform noise side by side
void generate_comparison_svg(
	const std::string &filename, int size, double scale, std::uint64_t seed,
	int octaves = 3, double persistence = 0.5
) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open output file: " << filename
				  << std::endl;
		return;
	}

	// Create noise generators
	istd::PerlinNoise raw_noise(seed);
	istd::UniformPerlinNoise uniform_noise(seed);

	// Calibrate uniform noise
	uniform_noise.calibrate(scale, octaves, persistence);

	const int pixel_size = 2;
	const int panel_width = size * pixel_size;
	const int svg_width = panel_width * 2 + 20; // Space for two panels + gap
	const int svg_height = size * pixel_size;

	// SVG header
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	file << "<svg width=\"" << svg_width << "\" height=\"" << svg_height
		 << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	file << "<title>Noise Comparison: Raw vs Uniform (Scale: " << scale
		 << ", Octaves: " << octaves << ", Seed: " << seed << ")</title>\n";

	// Generate histograms for statistics
	std::vector<int> raw_histogram(10, 0);
	std::vector<int> uniform_histogram(10, 0);

	// Generate left panel (raw noise)
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double noise_value = raw_noise.octave_noise(
				x * scale, y * scale, octaves, persistence
			);

			// Update histogram
			int bin = std::min(9, static_cast<int>(noise_value * 10));
			raw_histogram[bin]++;

			std::string color = noise_to_grayscale(noise_value);

			int svg_x = x * pixel_size;
			int svg_y = y * pixel_size;

			file << "<rect x=\"" << svg_x << "\" y=\"" << svg_y << "\" width=\""
				 << pixel_size << "\" height=\"" << pixel_size << "\" fill=\""
				 << color << "\"/>\n";
		}
	}

	// Generate right panel (uniform noise)
	int panel_offset = panel_width + 20;
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double noise_value = uniform_noise.uniform_noise(x, y);

			// Update histogram
			int bin = std::min(9, static_cast<int>(noise_value * 10));
			uniform_histogram[bin]++;

			std::string color = noise_to_grayscale(noise_value);

			int svg_x = panel_offset + x * pixel_size;
			int svg_y = y * pixel_size;

			file << "<rect x=\"" << svg_x << "\" y=\"" << svg_y << "\" width=\""
				 << pixel_size << "\" height=\"" << pixel_size << "\" fill=\""
				 << color << "\"/>\n";
		}
	}

	// Add labels
	file << "<text x=\"" << (panel_width / 2)
		 << "\" y=\"20\" font-family=\"Arial\" font-size=\"16\" "
	        "font-weight=\"bold\" text-anchor=\"middle\" fill=\"white\" "
	        "stroke=\"black\" stroke-width=\"1\">Raw Perlin Noise</text>\n";
	file << "<text x=\"" << (panel_offset + panel_width / 2)
		 << "\" y=\"20\" font-family=\"Arial\" font-size=\"16\" "
	        "font-weight=\"bold\" text-anchor=\"middle\" fill=\"white\" "
	        "stroke=\"black\" stroke-width=\"1\">Uniform Distribution</text>\n";

	// Add parameter info
	file << "<g transform=\"translate(10, " << (svg_height - 120) << ")\">\n";
	file << "<rect x=\"0\" y=\"0\" width=\"200\" height=\"100\" fill=\"white\" "
	        "stroke=\"black\" stroke-width=\"1\" opacity=\"0.9\"/>\n";
	file << "<text x=\"10\" y=\"20\" font-family=\"Arial\" font-size=\"12\" "
	        "font-weight=\"bold\">Parameters</text>\n";
	file << "<text x=\"10\" y=\"35\" font-family=\"Arial\" "
	        "font-size=\"10\">Size: "
		 << size << "x" << size << "</text>\n";
	file << "<text x=\"10\" y=\"50\" font-family=\"Arial\" "
	        "font-size=\"10\">Scale: "
		 << scale << "</text>\n";
	file << "<text x=\"10\" y=\"65\" font-family=\"Arial\" "
	        "font-size=\"10\">Octaves: "
		 << octaves << "</text>\n";
	file << "<text x=\"10\" y=\"80\" font-family=\"Arial\" "
	        "font-size=\"10\">Seed: "
		 << seed << "</text>\n";
	file << "</g>\n";

	file << "</svg>\n";
	file.close();

	std::cout << "Noise comparison SVG generated: " << filename << std::endl;
	std::cout << "Size: " << size << "x" << size << " pixels per panel"
			  << std::endl;
	std::cout << "Parameters: scale=" << scale << ", octaves=" << octaves
			  << ", seed=" << seed << std::endl;

	// Print histogram comparison
	std::cout << "\nValue Distribution Analysis:\n";
	std::cout << "Range    | Raw Noise | Uniform Noise\n";
	std::cout << "---------|-----------|--------------\n";
	for (int i = 0; i < 10; ++i) {
		double range_start = i * 0.1;
		double range_end = (i + 1) * 0.1;
		std::cout << std::fixed << std::setprecision(1) << range_start << "-"
				  << range_end << "  | " << std::setw(9) << raw_histogram[i]
				  << " | " << std::setw(12) << uniform_histogram[i] << "\n";
	}
}

int main(int argc, char *argv[]) {
	// Default parameters
	std::uint64_t seed = 12345;
	std::string output_filename = "noise_comparison.svg";
	double scale = 0.08;
	int octaves = 3;
	double persistence = 0.5;

	// Parse command line arguments
	if (argc >= 2) {
		seed = std::strtoull(argv[1], nullptr, 10);
	}
	if (argc >= 3) {
		output_filename = argv[2];
	}
	if (argc >= 4) {
		scale = std::strtod(argv[3], nullptr);
	}
	if (argc >= 5) {
		octaves = std::strtol(argv[4], nullptr, 10);
	}
	if (argc >= 6) {
		persistence = std::strtod(argv[5], nullptr);
	}

	if (argc == 1 || argc > 6) {
		std::cout << "Usage: " << argv[0]
				  << " [seed] [output.svg] [scale] [octaves] [persistence]\n";
		std::cout << "Defaults: seed=12345, output=noise_comparison.svg, "
		             "scale=0.08, octaves=3, persistence=0.5\n";
		std::cout << "This will generate a side-by-side comparison of raw vs "
		             "uniform Perlin noise\n";
		if (argc > 6) {
			return 1;
		}
	}

	std::cout << "Generating noise comparison (256x256 per panel)..."
			  << std::endl;
	std::cout << "Parameters: seed=" << seed << ", scale=" << scale
			  << ", octaves=" << octaves << ", persistence=" << persistence
			  << std::endl;

	// Generate the comparison
	generate_comparison_svg(
		output_filename, 256, scale, seed, octaves, persistence
	);

	return 0;
}
