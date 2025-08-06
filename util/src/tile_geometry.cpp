#include "istd_util/tile_geometry.h"

namespace istd {

// Amanatides-Woo Algorithm
std::generator<std::array<std::int32_t, 2>> tiles_on_segment(
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
		if (std::abs(t_max.x - t_max.y) < 1e-6f) {
			// Both directions are equal, choose one arbitrarily
			i += step_x;
			j += step_y;
			t_max.x += t_delta.x;
			t_max.y += t_delta.y;
		} else if (t_max.x < t_max.y) {
			i += step_x;
			t_max.x += t_delta.x;
		} else {
			j += step_y;
			t_max.y += t_delta.y;
		}
		co_yield {i, j};
	}
}

Vec2 tile_segment_intersection(
	Vec2 p1, Vec2 p2, std::array<std::int32_t, 2> tile
) noexcept {
	// Tile bounds: [i, i+1) x [j, j+1)
	float i = static_cast<float>(tile[0]);
	float j = static_cast<float>(tile[1]);
	float min_x = i, max_x = i + 1;
	float min_y = j, max_y = j + 1;

	// Parametric line: p = p1 + t * (p2 - p1), t in [0, 1]
	Vec2 d = p2 - p1;
	float t_min = 0.0f, t_max = 1.0f;

	// For each slab (x and y), compute intersection interval
	for (int axis = 0; axis < 2; ++axis) {
		float p = axis == 0 ? p1.x : p1.y;
		float q = axis == 0 ? d.x : d.y;
		float slab_min = axis == 0 ? min_x : min_y;
		float slab_max = axis == 0 ? max_x : max_y;
		if (std::abs(q) < 1e-8f) {
			// Parallel to slab, outside
			if (p < slab_min || p > slab_max) {
				return Vec2::invalid();
			}
		} else {
			float t1 = (slab_min - p) / q;
			float t2 = (slab_max - p) / q;
			if (t1 > t2) {
				std::swap(t1, t2);
			}
			t_min = std::max(t_min, t1);
			t_max = std::min(t_max, t2);
			if (t_min > t_max) {
				return Vec2::invalid();
			}
		}
	}
	// Intersection exists in [t_min, t_max], want closest to p1 (t_min >= 0)
	if (t_min < 0.0f || t_min > 1.0f) {
		return Vec2::invalid();
	}
	return p1 + d * t_min;
}

} // namespace istd
