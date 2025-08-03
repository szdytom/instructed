#include "chunk.h"
#include <cstdint>

namespace istd {

std::strong_ordering operator<=>(TilePos lhs, TilePos rhs) {
	if (lhs.chunk_x != rhs.chunk_x) {
		return lhs.chunk_x <=> rhs.chunk_x;
	}

	if (lhs.chunk_y != rhs.chunk_y) {
		return lhs.chunk_y <=> rhs.chunk_y;
	}

	if (lhs.local_x != rhs.local_x) {
		return lhs.local_x <=> rhs.local_x;
	}

	return lhs.local_y <=> rhs.local_y;
}

std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(SubChunkPos pos) {
	// Convert sub-chunk position to tile start coordinates
	return {pos.sub_x * Chunk::subchunk_size, pos.sub_y * Chunk::subchunk_size};
}

std::uint32_t TilePos::sqr_distance_to(TilePos other) const {
	auto [this_global_x, this_global_y] = to_global();
	auto [other_global_x, other_global_y] = other.to_global();

	using std::swap;
	if (this_global_x < other_global_x) {
		swap(this_global_x, other_global_x);
	}

	if (this_global_y < other_global_y) {
		swap(this_global_y, other_global_y);
	}

	std::uint32_t dx = this_global_x - other_global_x;
	std::uint32_t dy = this_global_y - other_global_y;
	return dx * dx + dy * dy;
}

std::pair<std::uint16_t, std::uint16_t> TilePos::to_global() const {
	return {chunk_x * Chunk::size + local_x, chunk_y * Chunk::size + local_y};
}

TilePos TilePos::from_global(std::uint16_t global_x, std::uint16_t global_y) {
	return {
		static_cast<uint8_t>(global_x / Chunk::size),
		static_cast<uint8_t>(global_y / Chunk::size),
		static_cast<uint8_t>(global_x % Chunk::size),
		static_cast<uint8_t>(global_y % Chunk::size)
	};
}

} // namespace istd
