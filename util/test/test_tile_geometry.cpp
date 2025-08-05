#include "istd_util/tile_geometry.h"
#include <cassert>
#include <tuple>
#include <vector>
using namespace istd;

int main() {
	// Test a simple horizontal segment
	Vec2 p1(0.5f, 1.2f);
	Vec2 p2(0.5f, 4.8f);
	std::vector<std::tuple<int, int>> result;
	for (auto [i, j] : tiles_on_segment(p1, p2)) {
		result.emplace_back(i, j);
	}
	// Should traverse columns 1 to 4, row 0
	assert(result.size() == 4);
	assert(result[0] == std::make_tuple(0, 1));
	assert(result[1] == std::make_tuple(0, 2));
	assert(result[2] == std::make_tuple(0, 3));
	assert(result[3] == std::make_tuple(0, 4));

	// Test a diagonal segment
	p1 = Vec2(1.1f, 1.1f);
	p2 = Vec2(3.9f, 3.9f);
	result.clear();
	for (auto [i, j] : tiles_on_segment(p1, p2)) {
		result.emplace_back(i, j);
	}
	// Should traverse (1,1), (2,2), (3,3)
	assert(result.size() == 3);
	assert(result[0] == std::make_tuple(1, 1));
	assert(result[1] == std::make_tuple(2, 2));
	assert(result[2] == std::make_tuple(3, 3));

	// Test vertical segment
	p1 = Vec2(2.2f, 0.5f);
	p2 = Vec2(5.7f, 0.5f);
	result.clear();
	for (auto [i, j] : tiles_on_segment(p1, p2)) {
		result.emplace_back(i, j);
	}
	// Should traverse rows 2 to 5, column 0
	assert(result.size() == 4);
	assert(result[0] == std::make_tuple(2, 0));
	assert(result[1] == std::make_tuple(3, 0));
	assert(result[2] == std::make_tuple(4, 0));
	assert(result[3] == std::make_tuple(5, 0));

	// Test single tile
	p1 = Vec2(7.3f, 8.9f);
	p2 = Vec2(7.7f, 8.1f);
	result.clear();
	for (auto [i, j] : tiles_on_segment(p1, p2)) {
		result.emplace_back(i, j);
	}
	assert(result.size() == 1);
	assert(result[0] == std::make_tuple(7, 8));

	return 0;
}
