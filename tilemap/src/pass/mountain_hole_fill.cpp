#include "generation.h"
#include <queue>

namespace istd {

MountainHoleFillPass::MountainHoleFillPass(const GenerationConfig &config)
	: config_(config) {}

void MountainHoleFillPass::operator()(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t total_tiles = map_size * Chunk::size;

	// Create visited array for the entire map
	std::vector<std::vector<bool>> visited(
		total_tiles, std::vector<bool>(total_tiles, false)
	);

	// Process all tiles in the map
	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			for (std::uint8_t local_x = 0; local_x < Chunk::size; ++local_x) {
				for (std::uint8_t local_y = 0; local_y < Chunk::size;
				     ++local_y) {
					TilePos pos{chunk_x, chunk_y, local_x, local_y};
					auto [global_x, global_y] = pos.to_global();

					// Skip if already visited
					if (visited[global_x][global_y]) {
						continue;
					}

					const Tile &tile = tilemap.get_tile(pos);

					// Only process passable tiles
					if (!is_passable(tile.base)) {
						visited[global_x][global_y] = true;
						continue;
					}

					// Find connected component
					std::vector<TilePos> component_positions;
					std::uint32_t component_size = bfs_component_size(
						tilemap, pos, visited, component_positions
					);

					// Check if this component touches the boundary
					bool touches_boundary = false;
					for (const auto component_pos : component_positions) {
						if (tilemap.is_at_boundary(component_pos)) {
							touches_boundary = true;
							break;
						}
					}

					// Fill small holes that don't touch the boundary
					if (!touches_boundary
					    && component_size <= config_.fill_threshold) {
						for (const TilePos &fill_pos : component_positions) {
							Tile fill_tile = tilemap.get_tile(fill_pos);
							fill_tile.base = BaseTileType::Mountain;
							tilemap.set_tile(fill_pos, fill_tile);
						}
					}
				}
			}
		}
	}
}

bool MountainHoleFillPass::is_passable(BaseTileType type) const {
	return type != BaseTileType::Mountain;
}

std::uint32_t MountainHoleFillPass::bfs_component_size(
	TileMap &tilemap, TilePos start_pos,
	std::vector<std::vector<bool>> &visited, std::vector<TilePos> &positions
) {
	std::queue<TilePos> queue;
	queue.push(start_pos);

	auto [start_global_x, start_global_y] = start_pos.to_global();
	visited[start_global_x][start_global_y] = true;

	std::uint32_t size = 0;
	positions.clear();

	while (!queue.empty()) {
		TilePos current = queue.front();
		queue.pop();
		positions.push_back(current);
		++size;

		// Check all neighbors
		std::vector<TilePos> neighbors = tilemap.get_neighbors(current);
		for (const auto neighbor : neighbors) {
			auto [neighbor_global_x, neighbor_global_y] = neighbor.to_global();
			if (visited[neighbor_global_x][neighbor_global_y]) {
				continue;
			}

			const Tile &neighbor_tile = tilemap.get_tile(neighbor);
			if (is_passable(neighbor_tile.base)) {
				visited[neighbor_global_x][neighbor_global_y] = true;
				queue.push(neighbor);
			}
		}
	}

	return size;
}

void TerrainGenerator::mountain_hole_fill_pass(TileMap &tilemap) {
	MountainHoleFillPass pass(config_);
	pass(tilemap);
}

} // namespace istd