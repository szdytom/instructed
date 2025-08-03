#ifndef ISTD_TILEMAP_XOROSHIRO_H
#define ISTD_TILEMAP_XOROSHIRO_H

#include <cstdint>
#include <limits>

namespace istd {

struct Seed {
	std::uint64_t s[2];

	static Seed from_string(const char *str);
	static Seed device_random();
};

/**
 * @brief Xoroshiro128++ random number generator.
 * @note This generator is not thread-safe and should not be used concurrently.
 * It is designed for high performance and provides a good quality of
 * randomness.
 * @link https://prng.di.unimi.it/xoroshiro128plusplus.c @endlink
 */
class Xoroshiro128PP {
public:
	Xoroshiro128PP() = default;
	Xoroshiro128PP(Seed seed);

	// Adaption for STL RandomEngine named requirements
	using result_type = std::uint64_t;
	static constexpr result_type min() {
		return std::numeric_limits<result_type>::min();
	}
	static constexpr result_type max() {
		return std::numeric_limits<result_type>::max();
	}
	result_type operator()(); // Equivalent to next(), for STL compatibility

	/**
	 * @brief Generates a random 64-bit unsigned integer.
	 * @return A random 64-bit unsigned integer.
	 * @note This function will modify the internal state of the generator.
	 * It is not thread-safe and should not be called concurrently.
	 * The generated number is uniformly distributed.
	 */
	std::uint64_t next();

	/**
	 * @brief Equivalent to 2^64 calls to next(), returns a new generator state.
	 * @return A new Xoroshiro128PP generator state.
	 * @note It can be used to generate two 2^64 non-overlapping sequences of
	 * random numbers for parallel processing.
	 */
	Xoroshiro128PP jump_64() const;

	/**
	 * @brief Equivalent to 2^96 calls to next(), returns a new generator state.
	 * @return A new Xoroshiro128PP generator state.
	 * @note It can be used to generate 2^32 starting points, from each of which
	 * `jump_64()` will generate 2^32 non-overlapping subsequences for parallel
	 * distributed computations.
	 */
	Xoroshiro128PP jump_96() const;

private:
	Seed seed;
};

} // namespace istd

#endif
