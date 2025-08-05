#include "istd_util/tile_geometry.h"

namespace istd {

// Amanatides-Woo Algorithm
std::generator<std::tuple<std::int32_t, std::int32_t>> tiles_on_segment(
	Vec2 p1, Vec2 p2
) noexcept {
	auto [i, j] = p1.floor();
	co_yield {i, j};
	if (p1.floor() == p2.floor()) {
		co_return;
	}

	auto delta = p2 - p1;
	int step_x = 0, step_y = 0;
	auto t_max = Vec2::inf(), t_delta = Vec2::inf();

	if (delta.x > 0) {
		step_x = 1;
		t_max.x = (i + 1 - p1.x) / delta.x;
		t_delta.x = 1.0f / delta.x;
	} else if (delta.x < 0) {
		step_x = -1;
		t_max.x = (i - p1.x) / delta.x;
		t_delta.x = -1.0f / delta.x;
	}

	if (delta.y > 0) {
		step_y = 1;
		t_max.y = (j + 1 - p1.y) / delta.y;
		t_delta.y = 1.0f / delta.y;
	} else if (delta.y < 0) {
		step_y = -1;
		t_max.y = (j - p1.y) / delta.y;
		t_delta.y = -1.0f / delta.y;
	}

	auto [end_i, end_j] = p2.floor();
	while (i != end_i || j != end_j) {
		if (t_max.x < t_max.y) {
			i += step_x;
			t_max.x += t_delta.x;
		} else {
			j += step_y;
			t_max.y += t_delta.y;
		}
		co_yield {i, j};
	}
}

} // namespace istd
