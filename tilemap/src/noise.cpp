#include "noise.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>

namespace istd {

PerlinNoise::PerlinNoise(Xoroshiro128PP rng) {
	// Initialize permutation array with values 0-255
	permutation_.resize(256);
	std::iota(permutation_.begin(), permutation_.end(), 0);

	// Shuffle using the provided seed
	std::shuffle(permutation_.begin(), permutation_.end(), rng);

	// Duplicate the permutation to avoid overflow
	permutation_.insert(
		permutation_.end(), permutation_.begin(), permutation_.end()
	);
}

double PerlinNoise::fade(double t) const {
	// Fade function: 6t^5 - 15t^4 + 10t^3
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const {
	return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y) const {
	// Convert low 4 bits of hash code into 12 gradient directions
	int h = hash & 15;
	double u = h < 8 ? x : y;
	double v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y) const {
	// Find unit grid cell containing point
	int X = static_cast<int>(std::floor(x)) & 255;
	int Y = static_cast<int>(std::floor(y)) & 255;

	// Get relative xy coordinates of point within that cell
	x -= std::floor(x);
	y -= std::floor(y);

	// Compute fade curves for each coordinate
	double u = fade(x);
	double v = fade(y);

	// Hash coordinates of the 4 cube corners
	int A = permutation_[X] + Y;
	int AA = permutation_[A];
	int AB = permutation_[A + 1];
	int B = permutation_[X + 1] + Y;
	int BA = permutation_[B];
	int BB = permutation_[B + 1];

	// Add blended results from 4 corners of the square
	double result = lerp(
		v,
		lerp(u, grad(permutation_[AA], x, y), grad(permutation_[BA], x - 1, y)),
		lerp(
			u, grad(permutation_[AB], x, y - 1),
			grad(permutation_[BB], x - 1, y - 1)
		)
	);

	// Convert from [-1,1] to [0,1]
	return (result + 1.0) * 0.5;
}

double PerlinNoise::octave_noise(
	double x, double y, int octaves, double persistence
) const {
	double value = 0.0;
	double amplitude = 1.0;
	double frequency = 1.0;
	double max_value = 0.0;

	for (int i = 0; i < octaves; i++) {
		value += noise(x * frequency, y * frequency) * amplitude;
		max_value += amplitude;
		amplitude *= persistence;
		frequency *= 2.0;
	}

	return value / max_value;
}

UniformPerlinNoise::UniformPerlinNoise(Xoroshiro128PP rng)
	: noise_(rng), calibrate_rng_(rng), is_calibrated_(false) {}

void UniformPerlinNoise::calibrate(
	double scale, int octaves, double persistence, int sample_size
) {
	scale_ = scale;
	octaves_ = octaves;
	persistence_ = persistence;

	// Clear previous calibration
	cdf_values_.clear();
	cdf_values_.reserve(sample_size);

	// Sample noise values across a reasonable range
	Xoroshiro128PP rng = calibrate_rng_;
	std::uniform_real_distribution<double> coord_dist(0.0, 1000.0);

	// Collect samples
	for (int i = 0; i < sample_size; ++i) {
		double x = coord_dist(rng);
		double y = coord_dist(rng);

		double noise_value;
		if (octaves_ == 1) {
			noise_value = noise_.noise(x * scale_, y * scale_);
		} else {
			noise_value = noise_.octave_noise(
				x * scale_, y * scale_, octaves_, persistence_
			);
		}

		cdf_values_.push_back(noise_value);
	}

	// Sort values to create CDF
	std::sort(cdf_values_.begin(), cdf_values_.end());

	is_calibrated_ = true;
}

double UniformPerlinNoise::uniform_noise(double x, double y) const {
	if (!is_calibrated_) {
		throw std::runtime_error(
			"UniformPerlinNoise must be calibrated before use"
		);
	}

	// Generate raw noise value
	double raw_value;
	if (octaves_ == 1) {
		raw_value = noise_.noise(x * scale_, y * scale_);
	} else {
		raw_value = noise_.octave_noise(
			x * scale_, y * scale_, octaves_, persistence_
		);
	}

	// Map to uniform distribution
	return map_to_uniform(raw_value);
}

double UniformPerlinNoise::map_to_uniform(double raw_value) const {
	// Find position in CDF using binary search
	auto it
		= std::lower_bound(cdf_values_.begin(), cdf_values_.end(), raw_value);

	// Calculate quantile (position in CDF as fraction)
	size_t position = std::distance(cdf_values_.begin(), it);
	double quantile = static_cast<double>(position) / cdf_values_.size();

	// Clamp to [0,1] range
	return std::max(0.0, std::min(1.0, quantile));
}

} // namespace istd
