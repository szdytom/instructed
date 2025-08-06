#include "istd_util/small_map.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace istd;

TEST_CASE("SmallMap basic operations", "[small_map]") {
	SECTION("insert and size") {
		SmallMap<int, int> map;
		REQUIRE(map.empty());

		map.insert(1, 10);
		map.insert(2, 20);
		map.insert(3, 30);

		REQUIRE(map.size() == 3);
		REQUIRE_FALSE(map.empty());
	}

	SECTION("operator[] access") {
		SmallMap<int, int> map;
		map.insert(1, 10);
		map.insert(2, 20);
		map.insert(3, 30);

		REQUIRE(map[1] == 10);
		REQUIRE(map[2] == 20);
		REQUIRE(map[3] == 30);
	}

	SECTION("erase operation") {
		SmallMap<int, int> map;
		map.insert(1, 10);
		map.insert(2, 20);
		map.insert(3, 30);

		map.erase(2);
		REQUIRE(map.size() == 2);
		REQUIRE(map[1] == 10);
		REQUIRE(map[3] == 30);
	}

	SECTION("clear operation") {
		SmallMap<int, int> map;
		map.insert(1, 10);
		map.insert(2, 20);

		map.clear();
		REQUIRE(map.empty());
	}
}

TEST_CASE("SmallMap exception handling", "[small_map]") {
	SECTION("duplicate insert throws") {
		SmallMap<int, int> map;
		map.insert(5, 50);

		REQUIRE_THROWS_AS(map.insert(5, 60), std::invalid_argument);
	}

	SECTION("out_of_range on operator[]") {
		SmallMap<int, int> map;
		map.insert(5, 50);

		REQUIRE_THROWS_AS(map[99], std::out_of_range);
	}

	SECTION("erase throws on missing key") {
		SmallMap<int, int> map;
		map.insert(5, 50);

		REQUIRE_THROWS_AS(map.erase(99), std::out_of_range);
	}
}

TEST_CASE("SmallMap iterators", "[small_map]") {
	SECTION("iterator traversal") {
		SmallMap<int, int> map;
		map.insert(5, 50);
		map.insert(6, 60);
		map.insert(7, 70);

		int sum = 0;
		for (auto it = map.begin(); it != map.end(); ++it) {
			sum += it->value;
		}
		REQUIRE(sum == 180); // 50 + 60 + 70
	}

	SECTION("const iterators") {
		SmallMap<int, int> map;
		map.insert(5, 50);
		map.insert(6, 60);
		map.insert(7, 70);

		const auto &cmap = map;
		int sum = 0;
		for (auto it = cmap.cbegin(); it != cmap.cend(); ++it) {
			sum += it->value;
		}
		REQUIRE(sum == 180);
	}
}
