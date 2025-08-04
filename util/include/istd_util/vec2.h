#ifndef ISTD_UTIL_VEC2_H
#define ISTD_UTIL_VEC2_H

#include <compare>
#include <stdexcept>
#include <tuple>

namespace istd {

/**
 * @brief 2D vector with float components.
 *
 * Provides basic arithmetic, comparison, and utility operations for 2D vectors.
 */
struct Vec2 {
	/**
	 * @brief X component of the vector.
	 */
	float x;
	/**
	 * @brief Y component of the vector.
	 */
	float y;

	/**
	 * @brief Construct a new Vec2 object.
	 * @param x X component
	 * @param y Y component
	 */
	Vec2(float x, float y) noexcept;
	/**
	 * @brief Returns a zero vector (0, 0).
	 */
	static Vec2 zero() noexcept;
	/**
	 * @brief Returns a vector rotated by the given angle.
	 * @param rad Angle in radians
	 * @param len Length of the resulting vector (default 1.0)
	 */
	static Vec2 rotated(float rad, float len = 1.0) noexcept;

	/**
	 * @name Symmetric operations
	 * @{
	 */
	/**
	 * @brief Vector addition.
	 */
	friend Vec2 operator+(Vec2 a, Vec2 b) noexcept;
	/**
	 * @brief Vector subtraction.
	 */
	friend Vec2 operator-(Vec2 a, Vec2 b) noexcept;
	/**
	 * @brief Unary negation.
	 */
	friend Vec2 operator-(Vec2 a) noexcept;
	/**
	 * @brief Scalar multiplication.
	 */
	friend Vec2 operator*(Vec2 a, float k) noexcept;
	/**
	 * @brief Scalar division.
	 */
	friend Vec2 operator/(Vec2 a, float k) noexcept;
	/**
	 * @brief Three-way comparison operator.
	 */
	friend std::strong_ordering operator<=>(Vec2 a, Vec2 b) noexcept;
	/** @} */

	/**
	 * @name Assignment operations
	 * @{
	 */
	/**
	 * @brief Adds another vector to this vector.
	 */
	Vec2 &operator+=(Vec2 b) noexcept;
	/**
	 * @brief Subtracts another vector from this vector.
	 */
	Vec2 &operator-=(Vec2 b) noexcept;
	/**
	 * @brief Multiplies this vector by a scalar.
	 */
	Vec2 &operator*=(float k) noexcept;
	/**
	 * @brief Divides this vector by a scalar.
	 */
	Vec2 &operator/=(float k) noexcept;
	/** @} */

	/**
	 * @brief Access vector components by index.
	 * @param i Index (0 for x, 1 for y)
	 * @return Reference to the component
	 * @throws std::out_of_range if index is not 0 or 1
	 */
	float &operator[](std::size_t i) {
		if (i == 0) {
			return x;
		}
		if (i == 1) {
			return y;
		}
		throw std::out_of_range("Index out of range for Vec2");
	}

	/**
	 * @brief Access vector components by index (const).
	 * @param i Index (0 for x, 1 for y)
	 * @return Value of the component
	 * @throws std::out_of_range if index is not 0 or 1
	 */
	float operator[](std::size_t i) const {
		if (i == 0) {
			return x;
		}
		if (i == 1) {
			return y;
		}
		throw std::out_of_range("Index out of range for Vec2");
	}

	/**
	 * @brief Returns the length (magnitude) of the vector.
	 */
	float length(this const Vec2 self) noexcept;
	/**
	 * @brief Returns the squared length of the vector.
	 */
	float length_squared(this const Vec2 self) noexcept;
	/**
	 * @brief Returns a normalized (unit length) vector.
	 */
	Vec2 normalized(this const Vec2 self) noexcept;
	/**
	 * @brief Returns a tuple of floored components.
	 */
	std::tuple<int, int> floor(this const Vec2 self) noexcept;
	/**
	 * @brief Returns a tuple of rounded components.
	 */
	std::tuple<int, int> round(this const Vec2 self) noexcept;

	/**
	 * @brief Returns the dot product of two vectors.
	 */
	friend float dot(Vec2 a, Vec2 b) noexcept;
	/**
	 * @brief Returns the cross product of two vectors.
	 */
	friend float cross(Vec2 a, Vec2 b) noexcept;
};

} // namespace istd

#endif
