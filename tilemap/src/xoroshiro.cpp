#include "xoroshiro.h"
#include <bit>
#include <initializer_list>
#include <random>

namespace istd {

Seed Seed::from_string(const char *str) {
	Seed res{0xfcc3a80ff25bae88, 0x78ac504431a5b8e6};
	constexpr std::uint64_t p1 = 0xb2209ed48ff3455b, p2 = 0x9f9a70d28f55f29f;
	for (int i = 0; str[i] != 0; ++i) {
		std::uint8_t c = str[i];
		res.s[0] ^= c;
		res.s[1] ^= c;
		res.s[0] *= p1;
		res.s[1] *= p2;
	}
	return res;
}

Seed Seed::device_random() {
	Seed seed;
	std::random_device rd;
	std::uniform_int_distribution<std::uint64_t> dist(
		std::numeric_limits<std::uint64_t>::min(),
		std::numeric_limits<std::uint64_t>::max()
	);
	seed.s[0] = dist(rd);
	seed.s[1] = dist(rd);
	return seed;
}

Xoroshiro128PP::Xoroshiro128PP(Seed seed): seed(seed) {}

Xoroshiro128PP::result_type Xoroshiro128PP::operator()() {
	return next();
}

std::uint64_t Xoroshiro128PP::next() {
	std::uint64_t s0 = seed.s[0];
	std::uint64_t s1 = seed.s[1];
	std::uint64_t result = std::rotl(s0 + s1, 17) + s0;

	s1 ^= s0;
	seed.s[0] = std::rotl(s0, 49) ^ s1 ^ (s1 << 21);
	seed.s[1] = std::rotl(s1, 28);

	return result;
}

Xoroshiro128PP Xoroshiro128PP::jump_64() const {
	constexpr std::uint64_t JUMP_64[]
		= {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c};

	Xoroshiro128PP res{seed};

	for (int i : {0, 1}) {
		for (int b = 0; b < 64; ++b) {
			if (JUMP_64[i] & (1ULL << b)) {
				res.seed.s[0] ^= seed.s[0];
				res.seed.s[1] ^= seed.s[1];
			}
			res.next();
		}
	}

	return res;
}

Xoroshiro128PP Xoroshiro128PP::jump_96() const {
	constexpr std::uint64_t JUMP_96[]
		= {0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3};

	Xoroshiro128PP res{seed};

	for (int i : {0, 1}) {
		for (int b = 0; b < 64; ++b) {
			if (JUMP_96[i] & (1ULL << b)) {
				res.seed.s[0] ^= seed.s[0];
				res.seed.s[1] ^= seed.s[1];
			}
			res.next();
		}
	}

	return res;
}

} // namespace istd