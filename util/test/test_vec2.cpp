#include "istd_util/vec2.h"
#include <cassert>
#include <cmath>
using namespace istd;

int main() {
	Vec2 v1(3.0f, 4.0f);
	Vec2 v2(1.0f, 2.0f);

	// Test length and length_squared
	assert(v1.length() == 5.0f);
	assert(v1.length_squared() == 25.0f);

	// Test normalized
	Vec2 n = v1.normalized();
	assert(std::abs(n.length() - 1.0f) < 1e-6);

	// Test addition and subtraction
	Vec2 v3 = v1 + v2;
	assert(v3.x == 4.0f && v3.y == 6.0f);
	Vec2 v4 = v1 - v2;
	assert(v4.x == 2.0f && v4.y == 2.0f);

	// Test floor and round
	Vec2 v5(1.7f, -2.3f);
	auto f = v5.floor();
	auto r = v5.round();
	assert(f == std::make_tuple(1, -3));
	assert(r == std::make_tuple(2, -2));

	// Test inf and invalid
	Vec2 vinf = Vec2::inf();
	assert(std::isinf(vinf.x) && std::isinf(vinf.y));
	Vec2 vinvalid = Vec2::invalid();
	assert(std::isnan(vinvalid.x) && std::isnan(vinvalid.y));

	// Test is_valid
	assert(v1.is_valid());
	assert(!vinvalid.is_valid());

	// Test static dot and cross
	assert(Vec2::dot(v1, v2) == 11.0f);
	assert(Vec2::cross(v1, v2) == 2.0f);
	return 0;
}
