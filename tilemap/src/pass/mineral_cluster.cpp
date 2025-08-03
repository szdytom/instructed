#include "tilemap/pass/mineral_cluster.h"
#include "tilemap/chunk.h"
#include "tilemap/generation.h"
#include "tilemap/noise.h"
#include "tilemap/xoroshiro.h"
#include <algorithm>
#include <queue>
#include <random>
#include <unordered_set>

namespace istd {

MineralClusterGenerationPass::MineralClusterGenerationPass(
	const GenerationConfig &config, Xoroshiro128PP rng, Xoroshiro128PP noise_rng
)
	: config_(config), rng_(rng), noise_(noise_rng) {}

void MineralClusterGenerationPass::operator()(TileMap &tilemap) {
	// Generate each mineral type with different densities
	const std::vector<std::pair<SurfaceTileType, std::uint16_t>> minerals = {
		{SurfaceTileType::Hematite,        config_.hematite_density       },
		{SurfaceTileType::Titanomagnetite, config_.titanomagnetite_density},
		{SurfaceTileType::Gibbsite,        config_.gibbsite_density       }
	};

	for (const auto [mineral_type, density] : minerals) {
		// Generate mineral centers using Poisson disk sampling approach
		auto mineral_centers = generate_mineral_centers(
			tilemap, mineral_type, density
		);

		// Generate mineral clusters around each center
		for (const auto &center : mineral_centers) {
			generate_mineral_cluster(tilemap, center, mineral_type);
		}
	}
}

std::vector<TilePos> MineralClusterGenerationPass::generate_mineral_centers(
	const TileMap &tilemap, SurfaceTileType mineral_type, std::uint16_t density
) {
	std::vector<TilePos> centers;
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t total_chunks = map_size * map_size;

	// Calculate expected number of mineral clusters based on density
	std::uint32_t expected_clusters = (total_chunks * density) / 255;

	// Minimum distance between mineral clusters to ensure spacing
	std::uint32_t min_distance = calculate_min_mineral_distance(density);

	const std::uint32_t max_coord = map_size * Chunk::size - 1;

	// Generate candidates with rejection sampling
	std::uint32_t attempts = 0;
	const std::uint32_t max_attempts = expected_clusters
		* 64; // More attempts for sparse minerals
	std::uniform_int_distribution<std::uint16_t> dist(0, max_coord);

	while (centers.size() < expected_clusters && attempts < max_attempts) {
		++attempts;

		// Generate random position
		const auto global_x = dist(rng_);
		const auto global_y = dist(rng_);

		TilePos candidate = TilePos::from_global(global_x, global_y);

		// Check if position is suitable for minerals (mountain edge)
		if (!is_suitable_for_mineral(tilemap, candidate)) {
			continue;
		}

		// Check distance to existing mineral centers
		auto distance_checker = [candidate, min_distance](TilePos existing) {
			return candidate.sqr_distance_to(existing)
				< (min_distance * min_distance);
		};
		if (std::ranges::any_of(centers, distance_checker)) {
			continue;
		}

		// Use base probability for mineral placement
		std::uint8_t sample = noise_.noise(
			global_x, global_y,
			static_cast<std::uint32_t>(
				mineral_type
			) // Use mineral type as seed variation
		);
		if (sample < config_.mineral_base_prob) {
			centers.push_back(candidate);
		}
	}

	return centers;
}

void MineralClusterGenerationPass::generate_mineral_cluster(
	TileMap &tilemap, TilePos center, SurfaceTileType mineral_type
) {
	auto [global_x, global_y] = center.to_global();

	// Calculate cluster size using similar approach to oil
	auto span = config_.mineral_cluster_max_size
		- config_.mineral_cluster_min_size;
	auto cluster_size = config_.mineral_cluster_min_size;

	// Use binomial distribution for cluster size
	for (int i = 1; i <= span; ++i) {
		auto sample = noise_.noise(
						  global_x, global_y,
						  i + static_cast<std::uint32_t>(mineral_type) * 16
					  )
			& 1;
		cluster_size += sample;
	}

	DiscreteRandomNoiseStream rng(
		noise_, global_x, global_y,
		64 + static_cast<std::uint32_t>(mineral_type) * 16
	);

	std::vector<TilePos> cluster_tiles;
	std::unordered_set<TilePos> visited;

	// Start with center if suitable
	cluster_tiles.push_back(center);
	visited.insert(center);

	// Grow cluster using random walk
	std::queue<TilePos> candidates;
	candidates.push(center);

	while (!candidates.empty() && cluster_tiles.size() < cluster_size) {
		TilePos current = candidates.front();
		candidates.pop();

		auto neighbors = tilemap.get_neighbors(current);
		std::shuffle(neighbors.begin(), neighbors.end(), rng);

		for (const auto neighbor : neighbors) {
			// 40% chance to skip this neighbor (slightly less dense than oil)
			auto [neighbor_global_x, neighbor_global_y] = neighbor.to_global();
			auto sample = noise_.noise(
				neighbor_global_x, neighbor_global_y,
				0x3c73dde4
					+ static_cast<std::uint32_t>(
						mineral_type
					)               // random seed per mineral
			);
			if ((sample % 5) < 2) { // 40% chance to skip
				continue;
			}

			if (visited.count(neighbor) > 0) {
				continue; // Already visited
			}

			if (!is_suitable_for_mineral(tilemap, neighbor)) {
				continue; // Not suitable for minerals
			}

			// Add to cluster
			cluster_tiles.push_back(neighbor);
			visited.insert(neighbor);

			// Stop if we reached the desired cluster size
			if (cluster_tiles.size() >= cluster_size) {
				break;
			}

			candidates.push(neighbor);
		}
	}

	// Place minerals on all cluster tiles
	for (const auto &pos : cluster_tiles) {
		Tile &tile = tilemap.get_tile(pos);
		tile.surface = mineral_type;
	}
}

bool MineralClusterGenerationPass::is_suitable_for_mineral(
	const TileMap &tilemap, TilePos pos
) const {
	const Tile &tile = tilemap.get_tile(pos);

	// Minerals can only be placed on mountains with empty surface
	if (tile.base != BaseTileType::Mountain
	    || tile.surface != SurfaceTileType::Empty) {
		return false;
	}

	// Must be on mountain edge (adjacent to non-mountain tiles)
	return is_mountain_edge(tilemap, pos);
}

bool MineralClusterGenerationPass::is_mountain_edge(
	const TileMap &tilemap, TilePos pos
) const {
	// Check if this mountain tile has at least one non-mountain neighbor
	auto neighbors = tilemap.get_neighbors(pos);

	for (const auto neighbor_pos : neighbors) {
		const Tile &neighbor_tile = tilemap.get_tile(neighbor_pos);
		if (neighbor_tile.base != BaseTileType::Mountain) {
			return true; // Found a non-mountain neighbor
		}
	}

	return false; // All neighbors are mountains, not an edge
}

std::uint32_t MineralClusterGenerationPass::calculate_min_mineral_distance(
	std::uint16_t density
) const {
	// Base distance on chunk size, but allow closer spacing than oil
	// since minerals are rarer and smaller clusters
	std::uint32_t base_distance = Chunk::size / 2;

	// Scale inversely with density, but with a minimum distance
	std::uint32_t scaled_distance = base_distance * 128
		/ std::max<std::uint16_t>(density, 1);

	// Ensure minimum distance of at least 8 tiles
	return std::max(scaled_distance, 8u);
}

void TerrainGenerator::mineral_cluster_pass(TileMap &tilemap) {
	auto rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	auto noise_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	MineralClusterGenerationPass pass(config_, rng, noise_rng);
	pass(tilemap);
}

} // namespace istd