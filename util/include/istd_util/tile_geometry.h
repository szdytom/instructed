#ifndef ISTD_UTIL_TILE_GEOMETRY_H
#define ISTD_UTIL_TILE_GEOMETRY_H

#include "istd_util/vec2.h"
#include <array>
#include <cstdint>
#include <generator>

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
std::generator<std::array<std::int32_t, 2>> tiles_on_segment(
	Vec2 p1, Vec2 p2
) noexcept;

/**
 * @brief Computes the first intersection point between a line segment and a
 * tile in the tilemap.
 *
 * Uses a coordinate system where x points downward (row index) and y points
 * rightward (column index). Finds the intersection point (closest to p1)
 * between the segment from p1 to p2 and the square tile specified by (i, j),
 * where the tile is defined as the region from (i, j) to (i+1, j+1).
 *
 * @param p1 The starting point of the segment (floating point coordinates).
 * @param p2 The ending point of the segment (floating point coordinates).
 * @param tile The tile indices (i, j) representing the square from (i, j) to
 * (i+1, j+1).
 * @return The intersection point as a Vec2, or an undefined value if there is
 * no intersection.
 */
Vec2 tile_segment_intersection(
	Vec2 p1, Vec2 p2, std::array<std::int32_t, 2> tile
) noexcept;

} // namespace istd

#endif