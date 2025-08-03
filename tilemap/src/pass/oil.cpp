#include "biome.h"
#include "chunk.h"
#include "generation.h"
#include "noise.h"
#include "xoroshiro.h"
#include <algorithm>
#include <queue>
#include <random>
#include <unordered_set>

namespace istd {

OilGenerationPass::OilGenerationPass(
	const GenerationConfig &config, Xoroshiro128PP rng, Xoroshiro128PP noise_rng
)
	: config_(config), rng_(rng), noise_(noise_rng) {}

void OilGenerationPass::operator()(TileMap &tilemap) {
	// Generate oil center positions using Poisson disk sampling
	auto oil_centers = generate_oil_centers(tilemap);

	// Generate oil clusters around each center
	for (const auto &center : oil_centers) {
		generate_oil_cluster(tilemap, center);
	}
}

std::vector<TilePos> OilGenerationPass::generate_oil_centers(
	const TileMap &tilemap
) {
	std::vector<TilePos> centers;
	std::uint8_t map_size = tilemap.get_size();
	std::uint32_t total_chunks = map_size * map_size;

	// Calculate expected number of oil fields based on density (out of 255)
	std::uint32_t expected_oil_fields = (total_chunks * config_.oil_density)
		/ 255;

	// Minimum distance between oil fields to ensure spacing
	std::uint32_t min_distance = calculate_min_oil_distance();

	// Use Poisson disk sampling approach
	const std::uint32_t max_coord = map_size * Chunk::size - 1;

	// Generate candidates with rejection sampling
	// Avoid infinite loops
	std::uint32_t attempts = 0;
	const std::uint32_t max_attempts = expected_oil_fields * 32;
	std::uniform_int_distribution<std::uint16_t> dist(0, max_coord);

	while (centers.size() < expected_oil_fields && attempts < max_attempts) {
		++attempts;

		// Generate random position using xoroshiro
		const auto global_x = dist(rng_);
		const auto global_y = dist(rng_);

		TilePos candidate = TilePos::from_global(global_x, global_y);

		// Check if position is suitable for oil
		if (!is_suitable_for_oil(tilemap, candidate)) {
			continue;
		}

		// Check distance to existing oil centers
		auto distance_checker = [candidate, min_distance](TilePos existing) {
			return candidate.sqr_distance_to(existing)
				< (min_distance * min_distance);
		};
		if (std::ranges::any_of(centers, distance_checker)) {
			continue;
		}

		// Apply biome preference
		const Chunk &chunk = tilemap.get_chunk_of(candidate);

		BiomeType biome = chunk.get_biome(candidate);
		std::uint8_t biome_preference = get_biome_oil_preference(biome);

		// Use integer probability check (0-255)
		std::uint8_t sample = noise_.noise(global_x, global_y);
		if (sample < biome_preference) {
			centers.push_back(candidate);
		}
	}

	return centers;
}

void OilGenerationPass::generate_oil_cluster(TileMap &tilemap, TilePos center) {
	auto [global_x, global_y] = center.to_global();

	auto span = config_.oil_cluster_max_size - config_.oil_cluster_min_size;
	auto cluster_size = config_.oil_cluster_min_size;
	// Binomial distribution for cluster size
	for (int i = 1; i <= span; ++i) {
		auto sample = noise_.noise(global_x, global_y, i) & 1;
		cluster_size += sample; // Increase cluster size based on noise
	}

	DiscreteRandomNoiseStream rng(noise_, global_x, global_y, 48);

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
			// 50% chance to skip this neighbor
			auto [neighbor_global_x, neighbor_global_y] = neighbor.to_global();
			auto sample = noise_.noise(
				neighbor_global_x, neighbor_global_y,
				0x2b52aaed // random seed
			);
			if ((sample & 1) == 0) {
				continue;
			}

			if (visited.count(neighbor) > 0) {
				continue; // Already visited
			}

			if (!is_suitable_for_oil(tilemap, neighbor)) {
				continue; // Not suitable for oil
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

	// Place oil on all cluster tiles
	for (const auto &pos : cluster_tiles) {
		Tile &tile = tilemap.get_tile(pos);
		tile.surface = SurfaceTileType::Oil;
	}
}

bool OilGenerationPass::is_suitable_for_oil(
	const TileMap &tilemap, TilePos pos
) const {
	const Tile &tile = tilemap.get_tile(pos);

	// Oil can only be placed on land or sand, and surface must be empty
	return (tile.base == BaseTileType::Land || tile.base == BaseTileType::Sand)
		&& tile.surface == SurfaceTileType::Empty;
}

std::uint8_t OilGenerationPass::get_biome_oil_preference(
	BiomeType biome
) const {
	std::uint8_t base_preference = config_.oil_base_probe;

	switch (biome) {
	case BiomeType::Desert:
	case BiomeType::Plains:
		return base_preference; // Full preference for desert/plains
	case BiomeType::Savanna:
	case BiomeType::SnowyPlains:
		return (base_preference * 204) / 255; // ~80% preference
	case BiomeType::Forest:
		return (base_preference * 128) / 255; // ~50% preference
	case BiomeType::SnowyPeeks:
		return (base_preference * 77) / 255;  // ~30% preference
	case BiomeType::FrozenOcean:
	case BiomeType::Ocean:
	case BiomeType::LukeOcean:
	default:
		return 0; // No oil in oceans
	}
}

std::uint32_t OilGenerationPass::calculate_min_oil_distance() const {
	// Base minimum distance on chunk size and oil density
	// Lower density = higher minimum distance
	std::uint32_t base_distance = Chunk::size * 4 / 5;
	return base_distance * 255 / config_.oil_density;
}

void TerrainGenerator::oil_pass(TileMap &tilemap) {
	auto rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	auto noise_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	OilGenerationPass pass(config_, rng, noise_rng);
	pass(tilemap);
}

} // namespace istd
