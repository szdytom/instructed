#ifndef ISTD_TILEMAP_NOISE_H
#define ISTD_TILEMAP_NOISE_H

#include <cstdint>
#include <vector>

namespace istd {

class PerlinNoise {
private:
	std::vector<int> permutation_;

	// Helper functions for Perlin noise calculation
	double fade(double t) const;
	double lerp(double t, double a, double b) const;
	double grad(int hash, double x, double y) const;

public:
	/**
	 * @brief Construct a PerlinNoise generator with the given seed
	 * @param seed Random seed for noise generation
	 */
	explicit PerlinNoise(std::uint64_t seed = 0);

	/**
	 * @brief Generate 2D Perlin noise value at the given coordinates
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @return Noise value between 0.0 and 1.0
	 */
	double noise(double x, double y) const;

	/**
	 * @brief Generate octave noise (multiple frequencies combined)
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @param octaves Number of octaves to combine
	 * @param persistence How much each octave contributes
	 * @return Noise value between 0.0 and 1.0
	 */
	double octave_noise(
		double x, double y, int octaves = 4, double persistence = 0.5
	) const;
};

} // namespace istd

#endif