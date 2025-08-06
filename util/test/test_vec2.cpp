#include "istd_util/vec2.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace istd;
using Catch::Approx;

TEST_CASE("Vec2 length operations", "[vec2]") {
	Vec2 v1(3.0f, 4.0f);

	SECTION("length calculation") {
		REQUIRE(v1.length() == 5.0f);
		REQUIRE(v1.length_squared() == 25.0f);
	}

	SECTION("normalized vector") {
		Vec2 n = v1.normalized();
		REQUIRE(n.length() == Approx(1.0f).epsilon(1e-6));
	}
}

TEST_CASE("Vec2 arithmetic operations", "[vec2]") {
	Vec2 v1(3.0f, 4.0f);
	Vec2 v2(1.0f, 2.0f);

	SECTION("addition") {
		Vec2 v3 = v1 + v2;
		REQUIRE(v3.x == 4.0f);
		REQUIRE(v3.y == 6.0f);
	}

	SECTION("subtraction") {
		Vec2 v4 = v1 - v2;
		REQUIRE(v4.x == 2.0f);
		REQUIRE(v4.y == 2.0f);
	}
}

TEST_CASE("Vec2 rounding operations", "[vec2]") {
	Vec2 v5(1.7f, -2.3f);

	SECTION("floor operation") {
		auto [i, j] = v5.floor();
		REQUIRE(i == 1);
		REQUIRE(j == -3);
	}

	SECTION("round operation") {
		auto [i, j] = v5.round();
		REQUIRE(i == 2);
		REQUIRE(j == -2);
	}
}

TEST_CASE("Vec2 special values", "[vec2]") {
	SECTION("infinity vector") {
		Vec2 vinf = Vec2::inf();
		REQUIRE(std::isinf(vinf.x));
		REQUIRE(std::isinf(vinf.y));
	}

	SECTION("invalid vector") {
		Vec2 vinvalid = Vec2::invalid();
		REQUIRE(std::isnan(vinvalid.x));
		REQUIRE(std::isnan(vinvalid.y));
	}

	SECTION("validity check") {
		Vec2 v1(3.0f, 4.0f);
		Vec2 vinvalid = Vec2::invalid();

		REQUIRE(v1.is_valid());
		REQUIRE_FALSE(vinvalid.is_valid());
	}
}

TEST_CASE("Vec2 static operations", "[vec2]") {
	Vec2 v1(3.0f, 4.0f);
	Vec2 v2(1.0f, 2.0f);

	SECTION("dot product") {
		REQUIRE(Vec2::dot(v1, v2) == 11.0f);
	}

	SECTION("cross product") {
		REQUIRE(Vec2::cross(v1, v2) == 2.0f);
	}
}
