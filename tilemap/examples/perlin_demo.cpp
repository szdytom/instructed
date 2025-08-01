#include "noise.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
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

// Generate SVG visualization of Perlin noise
void generate_noise_svg(
	const std::string &filename, int size, double scale, std::uint64_t seed,
	int octaves = 1, double persistence = 0.5
) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open output file: " << filename
				  << std::endl;
		return;
	}

	// Create noise generator
	istd::PerlinNoise noise(seed);

	const int pixel_size = 2; // Size of each pixel in SVG units
	const int svg_size = size * pixel_size;

	// SVG header
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	file << "<svg width=\"" << svg_size << "\" height=\"" << svg_size
		 << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	file << "<title>Perlin Noise Visualization (Scale: " << scale
		 << ", Octaves: " << octaves << ", Seed: " << seed << ")</title>\n";

	// Generate noise values and create rectangles
	double min_value = 1.0, max_value = 0.0;

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double noise_value;
			if (octaves == 1) {
				noise_value = noise.noise(x * scale, y * scale);
			} else {
				noise_value = noise.octave_noise(
					x * scale, y * scale, octaves, persistence
				);
			}

			// Track min/max for statistics
			min_value = std::min(min_value, noise_value);
			max_value = std::max(max_value, noise_value);

			std::string color = noise_to_grayscale(noise_value);

			int svg_x = x * pixel_size;
			int svg_y = y * pixel_size;

			file << "<rect x=\"" << svg_x << "\" y=\"" << svg_y << "\" width=\""
				 << pixel_size << "\" height=\"" << pixel_size << "\" fill=\""
				 << color << "\"/>\n";
		}
	}

	// Add information text
	file << "<!-- Statistics: Min=" << min_value << " Max=" << max_value
		 << " -->\n";

	// Add parameter info text overlay
	file << "<g transform=\"translate(10, 10)\">\n";
	file << "<rect x=\"0\" y=\"0\" width=\"300\" height=\"120\" fill=\"white\" "
	        "stroke=\"black\" stroke-width=\"1\" opacity=\"0.9\"/>\n";
	file << "<text x=\"10\" y=\"20\" font-family=\"Arial\" font-size=\"14\" "
	        "font-weight=\"bold\">Perlin Noise Parameters</text>\n";
	file << "<text x=\"10\" y=\"40\" font-family=\"Arial\" "
	        "font-size=\"12\">Size: "
		 << size << "x" << size << "</text>\n";
	file << "<text x=\"10\" y=\"55\" font-family=\"Arial\" "
	        "font-size=\"12\">Scale: "
		 << scale << "</text>\n";
	file << "<text x=\"10\" y=\"70\" font-family=\"Arial\" "
	        "font-size=\"12\">Seed: "
		 << seed << "</text>\n";
	file << "<text x=\"10\" y=\"85\" font-family=\"Arial\" "
	        "font-size=\"12\">Octaves: "
		 << octaves << "</text>\n";
	file << "<text x=\"10\" y=\"100\" font-family=\"Arial\" "
	        "font-size=\"12\">Range: ["
		 << std::fixed << std::setprecision(3) << min_value << ", " << max_value
		 << "]</text>\n";
	file << "</g>\n";

	// Add grayscale legend
	file << "<g transform=\"translate(" << (svg_size - 60) << ", 10)\">\n";
	file << "<text x=\"0\" y=\"15\" font-family=\"Arial\" font-size=\"12\" "
	        "font-weight=\"bold\">Value</text>\n";
	for (int i = 0; i <= 10; ++i) {
		double value = i / 10.0;
		std::string color = noise_to_grayscale(value);
		int y_pos = 20 + i * 15;
		file << "<rect x=\"0\" y=\"" << y_pos
			 << "\" width=\"20\" height=\"12\" fill=\"" << color
			 << "\" stroke=\"black\" stroke-width=\"0.5\"/>\n";
		file << "<text x=\"25\" y=\"" << (y_pos + 9)
			 << "\" font-family=\"Arial\" font-size=\"10\">" << std::fixed
			 << std::setprecision(1) << value << "</text>\n";
	}
	file << "</g>\n";

	file << "</svg>\n";
	file.close();

	std::cout << "Perlin noise SVG generated: " << filename << std::endl;
	std::cout << "Size: " << size << "x" << size << " pixels" << std::endl;
	std::cout << "Scale: " << scale << ", Octaves: " << octaves << std::endl;
	std::cout << "Value range: [" << std::fixed << std::setprecision(3)
			  << min_value << ", " << max_value << "]" << std::endl;
}

int main(int argc, char *argv[]) {
	// Default parameters
	std::uint64_t seed = 12345;
	std::string output_filename = "perlin_noise.svg";
	double scale = 0.02;
	int octaves = 1;
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
		std::cout << "Defaults: seed=12345, output=perlin_noise.svg, "
		             "scale=0.02, octaves=1, persistence=0.5\n";
		std::cout << "Examples:\n";
		std::cout << "  " << argv[0] << " 54321 noise1.svg 0.01\n";
		std::cout << "  " << argv[0] << " 12345 octave_noise.svg 0.02 4 0.5\n";
		if (argc > 6) {
			return 1;
		}
	}

	// Validate parameters
	if (scale <= 0) {
		std::cerr << "Error: Scale must be positive\n";
		return 1;
	}
	if (octaves < 1 || octaves > 10) {
		std::cerr << "Error: Octaves must be between 1 and 10\n";
		return 1;
	}
	if (persistence <= 0 || persistence > 1) {
		std::cerr << "Error: Persistence must be between 0 and 1\n";
		return 1;
	}

	std::cout << "Generating 256x256 Perlin noise visualization..."
			  << std::endl;
	std::cout << "Parameters: seed=" << seed << ", scale=" << scale
			  << ", octaves=" << octaves << ", persistence=" << persistence
			  << std::endl;

	// Generate the noise visualization
	generate_noise_svg(output_filename, 256, scale, seed, octaves, persistence);

	return 0;
}
