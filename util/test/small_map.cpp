#include "istd_util/small_map.h"
#include <cassert>

int main() {
	using namespace istd;
	// Test insert and size
	SmallMap<int, int> map;
	assert(map.empty());
	map.insert(1, 10);
	map.insert(2, 20);
	map.insert(3, 30);
	assert(map.size() == 3);
	assert(!map.empty());

	// Test operator[]
	assert(map[1] == 10);
	assert(map[2] == 20);
	assert(map[3] == 30);

	// Test erase
	map.erase(2);
	assert(map.size() == 2);
	assert(map[1] == 10);
	assert(map[3] == 30);

	// Test clear
	map.clear();
	assert(map.empty());

	// Test duplicate insert throws
	SmallMap<int, int> map2;
	map2.insert(5, 50);
	bool thrown = false;
	try {
		map2.insert(5, 60);
	} catch (const std::invalid_argument &) {
		thrown = true;
	}
	assert(thrown);

	// Test out_of_range on operator[]
	thrown = false;
	try {
		(void)map2[99];
	} catch (const std::out_of_range &) {
		thrown = true;
	}
	assert(thrown);

	// Test erase throws on missing key
	thrown = false;
	try {
		map2.erase(99);
	} catch (const std::out_of_range &) {
		thrown = true;
	}
	assert(thrown);

	// Test iterator
	map2.insert(6, 60);
	map2.insert(7, 70);
	int sum = 0;
	for (auto it = map2.begin(); it != map2.end(); ++it) {
		sum += it->value;
	}
	assert(sum == 180); // 50 + 60 + 70

	// Test const iterators
	const auto &cmap = map2;
	sum = 0;
	for (auto it = cmap.cbegin(); it != cmap.cend(); ++it) {
		sum += it->value;
	}
	assert(sum == 180);

	return 0;
}
