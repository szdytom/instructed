#include "bmp.h"
#include "noise.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// Generate comparison BMP showing both raw and uniform noise side by side
void generate_comparison_bmp(
	const std::string &filename, int size, double scale, std::uint64_t seed,
	int octaves = 3, double persistence = 0.5
) {
	// Create noise generators
	istd::PerlinNoise raw_noise(seed);
	istd::UniformPerlinNoise uniform_noise(seed);

	// Calibrate uniform noise
	uniform_noise.calibrate(scale, octaves, persistence);

	const int panel_width = size;
	const int gap = 10;
	const int total_width = panel_width * 2 + gap;

	BmpWriter bmp(total_width, size);

	// Fill gap with white
	for (int y = 0; y < size; ++y) {
		for (int x = panel_width; x < panel_width + gap; ++x) {
			bmp.set_pixel(x, y, 255, 255, 255);
		}
	}

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

			bmp.set_pixel_normalized(x, y, noise_value);
		}
	}

	// Generate right panel (uniform noise)
	int panel_offset = panel_width + gap;
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			double noise_value = uniform_noise.uniform_noise(x, y);

			// Update histogram
			int bin = std::min(9, static_cast<int>(noise_value * 10));
			uniform_histogram[bin]++;

			bmp.set_pixel_normalized(panel_offset + x, y, noise_value);
		}
	}

	if (!bmp.save(filename)) {
		std::cerr << "Error: Could not save BMP file: " << filename
				  << std::endl;
		return;
	}

	std::cout << "Noise comparison BMP generated: " << filename << std::endl;
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
	std::string output_filename = "noise_comparison.bmp";
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
				  << " [seed] [output.bmp] [scale] [octaves] [persistence]\n";
		std::cout << "Defaults: seed=12345, output=noise_comparison.bmp, "
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
	generate_comparison_bmp(
		output_filename, 256, scale, seed, octaves, persistence
	);

	return 0;
}
