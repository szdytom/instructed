#ifndef ISTD_UTIL_TILE_GEOMETRY_H
#define ISTD_UTIL_TILE_GEOMETRY_H

#include "istd_util/vec2.h"
#include <cstdint>
#include <generator>
#include <tuple>

namespace istd {

/**
 * @brief Iterates all tile coordinates traversed by a line segment on a
 * tilemap.
 *
 * Uses the Amanatides-Woo algorithm to efficiently enumerate all integer tile
 * positions that a segment from p1 to p2 passes through, including both
 * endpoints.
 *
 * @note x points downward, y points rightward,
 * i.e. x is row index, y is column index.
 *
 * @param p1 The starting point of the segment (floating point coordinates).
 * @param p2 The ending point of the segment (floating point coordinates).
 * @return Generator yielding (i, j) tuples for each tile crossed by the
 * segment.
 */
std::generator<std::tuple<std::int32_t, std::int32_t>> tiles_on_segment(
	Vec2 p1, Vec2 p2
) noexcept;

} // namespace istd

#endif