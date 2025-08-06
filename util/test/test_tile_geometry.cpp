#include "istd_util/tile_geometry.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <tuple>
#include <vector>

using namespace istd;
using Catch::Approx;

TEST_CASE("tiles_on_segment function", "[tile_geometry]") {
	SECTION("horizontal segment") {
		Vec2 p1(0.5f, 1.2f);
		Vec2 p2(0.5f, 4.8f);
		std::vector<std::tuple<int, int>> result;
		for (auto [i, j] : tiles_on_segment(p1, p2)) {
			result.emplace_back(i, j);
		}

		// Should traverse columns 1 to 4, row 0
		REQUIRE(result.size() == 4);
		REQUIRE(result[0] == std::make_tuple(0, 1));
		REQUIRE(result[1] == std::make_tuple(0, 2));
		REQUIRE(result[2] == std::make_tuple(0, 3));
		REQUIRE(result[3] == std::make_tuple(0, 4));
	}

	SECTION("diagonal segment") {
		Vec2 p1(1.1f, 1.1f);
		Vec2 p2(3.9f, 3.9f);
		std::vector<std::tuple<int, int>> result;
		for (auto [i, j] : tiles_on_segment(p1, p2)) {
			result.emplace_back(i, j);
		}

		// Should traverse (1,1), (2,2), (3,3)
		REQUIRE(result.size() == 3);
		REQUIRE(result[0] == std::make_tuple(1, 1));
		REQUIRE(result[1] == std::make_tuple(2, 2));
		REQUIRE(result[2] == std::make_tuple(3, 3));
	}

	SECTION("vertical segment") {
		Vec2 p1(2.2f, 0.5f);
		Vec2 p2(5.7f, 0.5f);
		std::vector<std::tuple<int, int>> result;
		for (auto [i, j] : tiles_on_segment(p1, p2)) {
			result.emplace_back(i, j);
		}

		// Should traverse rows 2 to 5, column 0
		REQUIRE(result.size() == 4);
		REQUIRE(result[0] == std::make_tuple(2, 0));
		REQUIRE(result[1] == std::make_tuple(3, 0));
		REQUIRE(result[2] == std::make_tuple(4, 0));
		REQUIRE(result[3] == std::make_tuple(5, 0));
	}

	SECTION("single tile") {
		Vec2 p1(7.3f, 8.9f);
		Vec2 p2(7.7f, 8.1f);
		std::vector<std::tuple<int, int>> result;
		for (auto [i, j] : tiles_on_segment(p1, p2)) {
			result.emplace_back(i, j);
		}

		REQUIRE(result.size() == 1);
		REQUIRE(result[0] == std::make_tuple(7, 8));
	}
}

TEST_CASE("tile_segment_intersection function", "[tile_geometry]") {
	SECTION("horizontal segment intersection") {
		Vec2 p1(0.5f, 1.2f);
		Vec2 p2(0.5f, 4.8f);
		Vec2 inter = tile_segment_intersection(p1, p2, {0, 2});

		REQUIRE(inter.is_valid());
		REQUIRE(inter.x == Approx(0.5f).epsilon(1e-6));
		REQUIRE(inter.y >= 2.0f);
		REQUIRE(inter.y <= 3.0f);
	}

	SECTION("diagonal segment intersection") {
		Vec2 p1(1.1f, 1.1f);
		Vec2 p2(3.9f, 3.9f);
		Vec2 inter = tile_segment_intersection(p1, p2, {2, 2});

		REQUIRE(inter.is_valid());
		REQUIRE(inter.x >= 2.0f);
		REQUIRE(inter.x <= 3.0f);
		REQUIRE(inter.y >= 2.0f);
		REQUIRE(inter.y <= 3.0f);
	}

	SECTION("no intersection") {
		Vec2 p1(0.0f, 0.0f);
		Vec2 p2(0.5f, 0.5f);
		Vec2 inter = tile_segment_intersection(p1, p2, {2, 2});

		REQUIRE_FALSE(inter.is_valid());
	}

	SECTION("segment starts inside tile") {
		Vec2 p1(2.2f, 2.2f);
		Vec2 p2(5.0f, 5.0f);
		Vec2 inter = tile_segment_intersection(p1, p2, {2, 2});

		REQUIRE(inter.is_valid());
		REQUIRE(inter.x == Approx(2.2f).epsilon(1e-6));
		REQUIRE(inter.y == Approx(2.2f).epsilon(1e-6));
	}
}
