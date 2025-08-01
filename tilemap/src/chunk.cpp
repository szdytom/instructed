#include "chunk.h"

namespace istd {

std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(SubChunkPos pos) {
	// Convert sub-chunk position to tile start coordinates
	return {pos.sub_x * Chunk::subchunk_size, pos.sub_y * Chunk::subchunk_size};
}

} // namespace istd
