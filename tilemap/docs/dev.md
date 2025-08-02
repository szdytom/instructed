# Tilemap Library Developer Guide

## Project Overview

The tilemap library is a C++ terrain generation system that creates tile-based worlds with biome support. It uses a multi-pass generation pipeline to create realistic, balanced terrain.

## Project Structure

```
tilemap/
├── include/           # Public headers
│   ├── tilemap.h     # Main map container
│   ├── chunk.h       # 64x64 tile chunks
│   ├── tile.h        # Individual tile types
│   ├── generation.h  # Generation system
│   ├── biome.h       # Biome system
│   ├── noise.h       # Noise generators
│   └── xoroshiro.h   # RNG implementation
├── src/              # Implementation files
│   ├── tilemap.cpp   # TileMap implementation
│   ├── chunk.cpp     # Chunk utilities
│   ├── generation.cpp # Main generation orchestrator
│   ├── biome.cpp     # Biome mapping logic
│   ├── noise.cpp     # Noise implementations
│   ├── xoroshiro.cpp # Xoroshiro128++ RNG
│   └── pass/         # Generation passes
│       ├── biome.cpp           # Climate-based biome generation
│       ├── base_tile_type.cpp  # Base terrain generation
│       ├── smoothen_mountain.cpp # Mountain smoothing
│       ├── mountain_hole_fill.cpp # Hole filling
│       └── deepwater.cpp       # Deep water placement
├── examples/         # Usage examples
└── docs/            # Documentation
```

## Core Architecture

### Data Organization

The system uses a hierarchical structure:
- **TileMap**: n×n grid of chunks
- **Chunk**: 64×64 tiles with biome metadata
- **Tile**: Individual terrain cell (1 byte packed)

Each chunk also contains a 16×16 grid of sub-chunk biomes, providing efficient biome lookup without per-tile storage.

### Pass-Based Generation

Terrain generation uses a multi-pass pipeline for modularity and control:

1. **Biome Pass**: Generates climate data and assigns biomes to sub-chunks
2. **Base Tile Type Pass**: Creates base terrain based on biomes
3. **Mountain Smoothing Pass**: Removes isolated mountain clusters
4. **Hole Fill Pass**: Fills small terrain holes
5. **Deep Water Pass**: Places deep water areas

Each pass operates independently with its own RNG state, ensuring deterministic results.

## Terrain Generation Pipeline

### Climate Generation

The biome pass uses dual noise generators for temperature and humidity:
- Separate Perlin noise for temperature/humidity at sub-chunk resolution
- Climate values mapped to 9 biome types in a 3×3 grid
- Sub-chunks store biome data for efficient terrain generation

### Noise System

The library addresses Perlin noise distribution issues:
- **Problem**: Raw Perlin noise has bell-curve distribution
- **Solution**: UniformPerlinNoise calibrates distribution to uniform [0,1]
- **Result**: Balanced terrain type ratios according to biome properties

### Terrain Generation Process

1. **Climate Sampling**: Sample temperature/humidity at sub-chunk centers
2. **Biome Assignment**: Map climate values to biome types
3. **Terrain Generation**: Generate tiles based on biome properties and noise
4. **Post-processing**: Apply smoothing and hole-filling algorithms

### Connected Component Analysis

Several passes use BFS (Breadth-First Search) for terrain analysis:
- **Mountain Smoothing**: Find and remove small mountain components
- **Hole Filling**: Identify and fill isolated terrain holes
- Components touching map boundaries are preserved

## Random Number Generation

### Xoroshiro128++ Implementation

High-quality PRNG with excellent statistical properties:
- 128-bit internal state
- Period of 2^128 - 1
- Jump functions for parallel generation
- STL-compatible interface

### Seed Management

128-bit seeds provide extensive randomness:
- String-based deterministic seed creation
- Hardware random seed generation
- Independent RNG streams for each generation pass

## Biome System

### Climate Mapping

Biomes are determined by temperature/humidity combinations:
```
       Dry    Moderate   Wet
Cold   Snowy  Snowy     Frozen
       Peaks  Plains    Ocean
Temp   Plains Forest    Ocean
Hot    Desert Savanna   Luke Ocean
```

### Biome Properties

Each biome defines terrain generation ratios:
- Water/Ice/Sand/Land ratios
- Noise parameters (octaves, persistence)
- Used by base terrain generation pass

## Performance Considerations

### Memory Layout

- Tiles packed into 1 byte (4 bits base + 4 bits surface)
- Chunks use contiguous 64×64 arrays for cache efficiency
- Sub-chunk biomes reduce memory overhead vs per-tile storage

### Generation Efficiency

- Sub-chunk resolution for biome generation (16×16 vs 64×64)
- Single-pass algorithms where possible
- Efficient connected component analysis using BFS

### Determinism

- Same seed produces identical results
- Independent RNG streams prevent cross-pass contamination
- Floating-point operations use consistent precision

## Adding New Generation Passes

To add a new generation pass:

1. **Create Pass Implementation**: Add new file in `src/pass/`
2. **Add to Pipeline**: Update `TerrainGenerator::operator()`
3. **RNG Management**: Use jump functions for independent RNG streams
4. **Configuration**: Add parameters to `GenerationConfig` if needed

Example pass structure:
```cpp
void TerrainGenerator::my_custom_pass(TileMap &tilemap) {
    auto rng = master_rng_.jump_96();  // Independent RNG stream
    
    // Process tilemap...
    for (auto chunk_y = 0; chunk_y < tilemap.get_size(); ++chunk_y) {
        for (auto chunk_x = 0; chunk_x < tilemap.get_size(); ++chunk_x) {
            auto& chunk = tilemap.get_chunk(chunk_x, chunk_y);
            // Process chunk...
        }
    }
}
```
