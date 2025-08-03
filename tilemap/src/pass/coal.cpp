#include "tilemap/pass/coal.h"
#include "tilemap/biome.h"
#include "tilemap/chunk.h"
#include "tilemap/generation.h"
#include "tilemap/noise.h"
#include "tilemap/xoroshiro.h"
#include <algorithm>
#include <queue>
#include <utility>

namespace istd {

CoalGenerationPass::CoalGenerationPass(
	const GenerationConfig &config, Xoroshiro128PP rng, Xoroshiro128PP noise_rng
)
	: config_(config), rng_(rng), noise_(noise_rng) {}

void CoalGenerationPass::operator()(TileMap &tilemap) {
	std::uint8_t map_size = tilemap.get_size();
	std::vector<TilePos> all_seeds;

	// Generate coal seeds for each chunk
	for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
		for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
			chunk_coal_seeds(tilemap, chunk_x, chunk_y, all_seeds);
		}
	}

	// Evolve coal deposits using cellular automata
	evolve_coal_deposits(tilemap, all_seeds);
}

void CoalGenerationPass::chunk_coal_seeds(
	const TileMap &tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
	std::vector<TilePos> &seeds
) {
	// Use a max heap to keep top N positions by noise value
	using PosNoisePair = std::pair<std::uint64_t, TilePos>;
	std::priority_queue<PosNoisePair> heap;

	for (std::uint8_t local_x = 0; local_x < Chunk::size; ++local_x) {
		for (std::uint8_t local_y = 0; local_y < Chunk::size; ++local_y) {
			TilePos candidate{chunk_x, chunk_y, local_x, local_y};
			if (!is_suitable_for_coal(tilemap, candidate)) {
				continue;
			}

			auto [global_x, global_y] = candidate.to_global();
			auto noise_val = noise_.noise(global_x, global_y, 0x90);

			heap.emplace(noise_val, candidate);
			if (heap.size() > config_.coal_seeds_per_chunk) {
				heap.pop();
			}
		}
	}

	while (!heap.empty()) {
		seeds.push_back(heap.top().second);
		heap.pop();
	}
}

void CoalGenerationPass::evolve_coal_deposits(
	TileMap &tilemap, const std::vector<TilePos> &initial_seeds
) {
	// Place initial seeds
	for (const auto &seed : initial_seeds) {
		Tile &tile = tilemap.get_tile(seed);
		if (tile.surface == SurfaceTileType::Empty) {
			tile.surface = SurfaceTileType::Coal;
		}
	}

	// Evolve using cellular automata
	for (std::uint8_t step = 1; step <= config_.coal_evolution_steps; ++step) {
		std::vector<TilePos> new_coal_positions;
		std::uint8_t map_size = tilemap.get_size();

		// Iterate through all tiles
		for (std::uint8_t chunk_x = 0; chunk_x < map_size; ++chunk_x) {
			for (std::uint8_t chunk_y = 0; chunk_y < map_size; ++chunk_y) {
				const Chunk &chunk = tilemap.get_chunk(chunk_x, chunk_y);

				for (std::uint8_t local_x = 0; local_x < Chunk::size;
				     ++local_x) {
					for (std::uint8_t local_y = 0; local_y < Chunk::size;
					     ++local_y) {
						TilePos pos{chunk_x, chunk_y, local_x, local_y};
						const Tile &tile = tilemap.get_tile(pos);

						// Skip if not suitable for coal
						if (!is_suitable_for_coal(tilemap, pos)) {
							continue;
						}

						// Skip if already has coal
						if (tile.surface == SurfaceTileType::Coal) {
							continue;
						}

						// Count coal neighbors
						std::uint8_t coal_neighbors = count_coal_neighbors(
							tilemap, pos
						);

						if (coal_neighbors > 0) {
							// Get biome for this position
							BiomeType biome = chunk.get_biome(local_x, local_y);
							auto biome_probability
								= get_biome_coal_growth_probability(biome);

							// Base probability increases with more coal
							// neighbors
							std::uint32_t base_probability = coal_neighbors
								* config_.coal_growth_base_prob;
							std::uint8_t final_probability = std::clamp(
								base_probability * biome_probability / 255, 0u,
								255u
							);

							// Use noise to decide whether to grow coal here
							auto [global_x, global_y] = pos.to_global();
							std::uint8_t sample = 0xFF
								& noise_.noise(global_x, global_y, step);

							if (sample < final_probability) {
								new_coal_positions.push_back(pos);
							}
						}
					}
				}
			}
		}

		// Place new coal
		for (const auto &pos : new_coal_positions) {
			Tile &tile = tilemap.get_tile(pos);
			if (tile.surface == SurfaceTileType::Empty) {
				tile.surface = SurfaceTileType::Coal;
			}
		}
	}
}

bool CoalGenerationPass::is_suitable_for_coal(
	const TileMap &tilemap, TilePos pos
) const {
	const Tile &tile = tilemap.get_tile(pos);
	return (tile.base == BaseTileType::Sand || tile.base == BaseTileType::Land)
		&& tile.surface == SurfaceTileType::Empty;
}

// Returns the coal growth probability for a given biome.
// Higher values mean coal is more likely to spread in that biome.
std::uint8_t CoalGenerationPass::get_biome_coal_growth_probability(
	BiomeType biome
) const {
	switch (biome) {
	case BiomeType::Forest:
		return 255;
	case BiomeType::LukeOcean:
		return 204;
	case BiomeType::Savanna:
		return 153;
	case BiomeType::Plains:
		return 128;
	case BiomeType::SnowyPlains:
	case BiomeType::Ocean:
		return 102;
	case BiomeType::SnowyPeeks:
		return 77;
	case BiomeType::Desert:
		return 51;
	case BiomeType::FrozenOcean:
		return 26;
	default:
		std::unreachable();
	}
}

std::uint8_t CoalGenerationPass::count_coal_neighbors(
	const TileMap &tilemap, TilePos pos
) const {
	std::uint8_t count = 0;
	auto neighbors = tilemap.get_neighbors(pos);

	for (const auto neighbor_pos : neighbors) {
		const Tile &neighbor_tile = tilemap.get_tile(neighbor_pos);
		if (neighbor_tile.surface == SurfaceTileType::Coal) {
			++count;
		}
	}

	return count;
}

void TerrainGenerator::coal_pass(TileMap &tilemap) {
	auto rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	auto noise_rng = master_rng_;
	master_rng_ = master_rng_.jump_96();
	CoalGenerationPass pass(config_, rng, noise_rng);
	pass(tilemap);
}

} // namespace istd
