#include "bmp.h"
#include "noise.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

// Generate BMP file from Perlin noise
void generate_noise_bmp(
	const std::string &filename, int size, double scale, std::uint64_t seed,
	int octaves = 1, double persistence = 0.5
) {
	// Create noise generator
	istd::PerlinNoise noise(seed);

	BmpWriter bmp(size, size);

	// Generate noise values and statistics
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

			bmp.set_pixel_normalized(x, y, noise_value);
		}
	}

	if (!bmp.save(filename)) {
		std::cerr << "Error: Could not save BMP file: " << filename
				  << std::endl;
		return;
	}

	std::cout << "Perlin noise BMP generated: " << filename << std::endl;
	std::cout << "Size: " << size << "x" << size << " pixels" << std::endl;
	std::cout << "Scale: " << scale << ", Octaves: " << octaves << std::endl;
	std::cout << "Value range: [" << std::fixed << std::setprecision(3)
			  << min_value << ", " << max_value << "]" << std::endl;
}

int main(int argc, char *argv[]) {
	// Default parameters
	std::uint64_t seed = 12345;
	std::string output_filename = "perlin_noise.bmp";
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
				  << " [seed] [output.bmp] [scale] [octaves] [persistence]\n";
		std::cout << "Defaults: seed=12345, output=perlin_noise.bmp, "
		             "scale=0.02, octaves=1, persistence=0.5\n";
		std::cout << "Examples:\n";
		std::cout << "  " << argv[0] << " 54321 noise1.bmp 0.01\n";
		std::cout << "  " << argv[0] << " 12345 octave_noise.bmp 0.02 4 0.5\n";
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
	generate_noise_bmp(output_filename, 256, scale, seed, octaves, persistence);

	return 0;
}
