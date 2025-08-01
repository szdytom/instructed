#include "noise.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace istd {

PerlinNoise::PerlinNoise(std::uint64_t seed) {
	// Initialize permutation array with values 0-255
	permutation_.resize(256);
	std::iota(permutation_.begin(), permutation_.end(), 0);

	// Shuffle using the provided seed
	std::default_random_engine generator(seed);
	std::shuffle(permutation_.begin(), permutation_.end(), generator);

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

} // namespace istd
