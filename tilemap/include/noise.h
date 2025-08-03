#ifndef ISTD_TILEMAP_NOISE_H
#define ISTD_TILEMAP_NOISE_H

#include "xoroshiro.h"
#include <array>
#include <cstdint>
#include <vector>

namespace istd {

/**
 * @brief Discrete random noise generator for terrain replacement operations
 *
 * Provides high-quality discrete random values based on Xoroshiro128++ RNG.
 * Used for selecting terrain types during mountain smoothing operations.
 */
class DiscreteRandomNoise {
private:
	std::uint64_t mask;
	std::array<std::uint8_t, 256> permutation_;

	std::uint8_t perm(int x) const noexcept;
	std::uint32_t rot8(std::uint32_t x) const noexcept;
	std::uint32_t map_once(std::uint32_t x) const noexcept;
	std::uint32_t map(std::uint32_t x) const noexcept;

public:
	/**
	 * @brief Construct a DiscreteRandomNoise generator with the given seed
	 * @param rng Random number generator for noise
	 */
	explicit DiscreteRandomNoise(Xoroshiro128PP rng) noexcept;

	/**
	 * @brief Generate a discrete random value at the given coordinates
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @param z Z coordinate (optional)
	 * @return Discrete random value between 0 and 2^64-1
	 */
	std::uint64_t noise(
		std::uint32_t x, std::uint32_t y, std::uint32_t z = 0
	) const noexcept;
};

class DiscreteRandomNoiseStream {
private:
	const DiscreteRandomNoise &noise_;
	std::uint32_t x_;
	std::uint32_t y_;
	std::uint32_t idx_;

public:
	DiscreteRandomNoiseStream(
		const DiscreteRandomNoise &noise, std::uint32_t x, std::uint32_t y,
		std::uint32_t idx = 0
	);

	std::uint64_t next() noexcept;

	// Adaption for STL RandomEngine named requirements
	using result_type = std::uint64_t;
	static constexpr result_type min() noexcept {
		return std::numeric_limits<result_type>::min();
	}
	static constexpr result_type max() noexcept {
		return std::numeric_limits<result_type>::max();
	}
	// Equivalent to next(), for STL compatibility
	result_type operator()() noexcept;
};

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
	 * @param rng Random number generator for noise
	 */
	explicit PerlinNoise(Xoroshiro128PP rng);

	PerlinNoise() = default;

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
		double x, double y, int octaves, double persistence
	) const;
};

/**
 * @brief A wrapper that provides uniform distribution mapping for Perlin noise
 *
 * This class samples the noise distribution and builds a CDF (Cumulative
 * Distribution Function) to map the non-uniform Perlin noise values to a
 * uniform [0,1] distribution using quantiles.
 */
class UniformPerlinNoise {
private:
	PerlinNoise noise_;
	Xoroshiro128PP calibrate_rng_;
	std::vector<double> cdf_values_; // Sorted noise values for CDF
	bool is_calibrated_;

	// Parameters used for calibration
	double scale_;
	int octaves_;
	double persistence_;

public:
	/**
	 * @brief Construct a UniformPerlinNoise generator
	 * @param seed Random seed for noise generation
	 */
	explicit UniformPerlinNoise(Xoroshiro128PP rng);

	UniformPerlinNoise() = default;

	/**
	 * @brief Calibrate the noise distribution by sampling
	 * @param scale The scale parameter that will be used for generation
	 * @param octaves Number of octaves for octave noise
	 * @param persistence Persistence for octave noise
	 * @param sample_size Number of samples to use for CDF (default: 10000)
	 */
	void calibrate(
		double scale, int octaves, double persistence, int sample_size = 10000
	);

	/**
	 * @brief Generate uniform noise value at the given coordinates
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @return Uniformly distributed noise value between 0.0 and 1.0
	 * @note Must call calibrate() first
	 */
	double uniform_noise(double x, double y) const;

	/**
	 * @brief Check if the noise generator has been calibrated
	 */
	bool is_calibrated() const {
		return is_calibrated_;
	}

private:
	/**
	 * @brief Map a raw noise value to uniform distribution using the CDF
	 * @param raw_value Raw noise value from Perlin noise
	 * @return Uniformly distributed value between 0.0 and 1.0
	 */
	double map_to_uniform(double raw_value) const;
};

} // namespace istd

#endif