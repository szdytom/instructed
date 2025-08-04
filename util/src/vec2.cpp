
#include "istd_util/vec2.h"
#include <cmath>

namespace istd {

Vec2::Vec2(float x_, float y_) noexcept: x(x_), y(y_) {}

Vec2 Vec2::zero() noexcept {
	return Vec2(0.0f, 0.0f);
}

Vec2 Vec2::rotated(float rad, float len) noexcept {
	return Vec2(std::cos(rad) * len, std::sin(rad) * len);
}

Vec2 operator+(Vec2 a, Vec2 b) noexcept {
	return Vec2(a.x + b.x, a.y + b.y);
}

Vec2 operator-(Vec2 a, Vec2 b) noexcept {
	return Vec2(a.x - b.x, a.y - b.y);
}

Vec2 operator-(Vec2 a) noexcept {
	return Vec2(-a.x, -a.y);
}

Vec2 operator*(Vec2 a, float k) noexcept {
	return Vec2(a.x * k, a.y * k);
}

Vec2 operator/(Vec2 a, float k) noexcept {
	return Vec2(a.x / k, a.y / k);
}

std::strong_ordering operator<=>(Vec2 a, Vec2 b) noexcept {
	if (a.x < b.x) {
		return std::strong_ordering::less;
	}
	if (a.x > b.x) {
		return std::strong_ordering::greater;
	}
	if (a.y < b.y) {
		return std::strong_ordering::less;
	}
	if (a.y > b.y) {
		return std::strong_ordering::greater;
	}
	return std::strong_ordering::equal;
}

Vec2 &Vec2::operator+=(Vec2 b) noexcept {
	x += b.x;
	y += b.y;
	return *this;
}

Vec2 &Vec2::operator-=(Vec2 b) noexcept {
	x -= b.x;
	y -= b.y;
	return *this;
}

Vec2 &Vec2::operator*=(float k) noexcept {
	x *= k;
	y *= k;
	return *this;
}

Vec2 &Vec2::operator/=(float k) noexcept {
	x /= k;
	y /= k;
	return *this;
}

float Vec2::length(this const Vec2 self) noexcept {
	return std::sqrt(self.x * self.x + self.y * self.y);
}

float Vec2::length_squared(this const Vec2 self) noexcept {
	return self.x * self.x + self.y * self.y;
}

Vec2 Vec2::normalized(this const Vec2 self) noexcept {
	float len = self.length();
	if (len == 0.0f) {
		return Vec2(0.0f, 0.0f);
	}
	return Vec2(self.x / len, self.y / len);
}

std::tuple<int, int> Vec2::floor(this const Vec2 self) noexcept {
	return std::make_tuple(
		static_cast<int>(std::floor(self.x)),
		static_cast<int>(std::floor(self.y))
	);
}

std::tuple<int, int> Vec2::round(this const Vec2 self) noexcept {
	return std::make_tuple(
		static_cast<int>(std::round(self.x)),
		static_cast<int>(std::round(self.y))
	);
}

float dot(Vec2 a, Vec2 b) noexcept {
	return a.x * b.x + a.y * b.y;
}

float cross(Vec2 a, Vec2 b) noexcept {
	return a.x * b.y - a.y * b.x;
}

} // namespace istd
