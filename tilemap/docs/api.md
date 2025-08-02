# Tilemap Library API Documentation

## Overview

The tilemap library provides a flexible system for generating and managing tile-based terrain with biome support. The library consists of several main components:

- **TileMap**: The main map container holding chunks of tiles
- **Chunk**: 64x64 tile containers with biome information
- **Tile**: Individual map tiles with base and surface types
- **TerrainGenerator**: Pass-based procedural terrain generation system
- **Generation Passes**: Modular generation components (biome, base terrain)
- **Biome System**: Climate-based terrain variation

## Core Classes

### TileMap

The main container for the entire map, organized as an n×n grid of chunks.

```cpp
class TileMap {
public:
    explicit TileMap(std::uint8_t size);

    std::uint8_t get_size() const;
    Chunk& get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y);
    const Chunk& get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y) const;

    Tile& get_tile(TilePos pos);
    const Tile& get_tile(TilePos pos) const;
    void set_tile(TilePos pos, const Tile& tile);
};
```

**Constructor Parameters:**
- `size`: Number of chunks per side (max 100), creating an n×n grid

### Chunk

Each chunk contains 64×64 tiles and sub-chunk biome information.

```cpp
struct Chunk {
    static constexpr uint8_t size = 64;           // Tiles per side
    static constexpr uint8_t subchunk_size = /*default value*/;   // Tiles per sub-chunk side
    static constexpr uint8_t subchunk_count = size / subchunk_size;  // Sub-chunks per side

    Tile tiles[size][size];                       // 64x64 tile grid
    BiomeType biome[subchunk_count][subchunk_count]; // Sub-chunk biomes

    // Get biome for a specific sub-chunk position
    BiomeType& get_biome(SubChunkPos pos);
    const BiomeType& get_biome(SubChunkPos pos) const;
};
```

### Tile

Individual map tiles with base terrain and surface features.

```cpp
struct Tile {
    BaseTileType base : 4;      // Base terrain type
    SurfaceTileType surface : 4; // Surface features
};
```

**Base Tile Types:**
- `Land`: Standard ground terrain
- `Mountain`: Rocky elevated terrain
- `Sand`: Desert/beach terrain
- `Water`: Water bodies
- `Ice`: Frozen terrain

**Surface Tile Types:**
- `Empty`: No surface features
- `Wood`: Trees/vegetation
- `Structure`: Player-built structures

### TilePos

Position structure for locating tiles within the map.

```cpp
struct TilePos {
    uint8_t chunk_x;  // Chunk X coordinate
    uint8_t chunk_y;  // Chunk Y coordinate
    uint8_t local_x;  // Tile X within chunk (0-63)
    uint8_t local_y;  // Tile Y within chunk (0-63)
};
```

### SubChunkPos

Position within a chunk's sub-chunk grid.

```cpp
struct SubChunkPos {
    std::uint8_t sub_x;
    std::uint8_t sub_y;
    
    constexpr SubChunkPos(std::uint8_t x, std::uint8_t y);
};

std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(
    SubChunkPos pos
);
```

## Terrain Generation

The terrain generation system has been refactored into a modular pass-based architecture, providing better separation of concerns and more flexible generation control.

### GenerationConfig

Configuration parameters for terrain generation.

```cpp
struct GenerationConfig {
    Seed seed;                                    // 128-bit seed for random generation

    // Temperature noise parameters
    double temperature_scale = /*default value*/;             // Scale for temperature noise
    int temperature_octaves = /*default value*/;                 // Number of octaves for temperature noise
    double temperature_persistence = /*default value*/;       // Persistence for temperature noise

    // Humidity noise parameters
    double humidity_scale = /*default value*/;               // Scale for humidity noise
    int humidity_octaves = /*default value*/;                   // Number of octaves for humidity noise
    double humidity_persistence = /*default value*/;         // Persistence for humidity noise

    // Base terrain noise parameters
    double base_scale = /*default value*/;                   // Scale for base terrain noise
    int base_octaves = /*default value*/;                       // Number of octaves for base terrain noise
    double base_persistence = /*default value*/;             // Persistence for base terrain noise
};
```

**Parameters:**

- `seed`: 128-bit seed for all noise generators (see Seed structure)
- `temperature_scale`: Controls the scale/frequency of temperature variation across the map
- `temperature_octaves`: Number of noise octaves for temperature (more octaves = more detail)
- `temperature_persistence`: How much each octave contributes to temperature noise (0.0-1.0)
- `humidity_scale`: Controls the scale/frequency of humidity variation across the map
- `humidity_octaves`: Number of noise octaves for humidity
- `humidity_persistence`: How much each octave contributes to humidity noise (0.0-1.0)
- `base_scale`: Controls the scale/frequency of base terrain height variation
- `base_octaves`: Number of noise octaves for base terrain
- `base_persistence`: How much each octave contributes to base terrain noise (0.0-1.0)

### Generation Passes

The generation system is organized into distinct passes, each responsible for a specific aspect of terrain generation.

#### BiomeGenerationPass

Generates biome data for all sub-chunks based on temperature and humidity noise.

```cpp
class BiomeGenerationPass {
public:
    BiomeGenerationPass(
        const GenerationConfig& config, 
        Xoroshiro128PP r1, 
        Xoroshiro128PP r2
    );
    
    void operator()(TileMap& tilemap);

private:
    std::pair<double, double> get_climate(double global_x, double global_y) const;
};
```

**Key Features:**
- **Climate Generation**: Uses separate noise generators for temperature and humidity
- **Sub-chunk Resolution**: Assigns biomes to 16×16 sub-chunks for efficient generation
- **Climate Mapping**: Maps noise values to temperature/humidity ranges
- **Biome Determination**: Uses climate values to determine appropriate biomes

#### BaseTileTypeGenerationPass

Generates base terrain types for all tiles based on their sub-chunk biomes.

```cpp
class BaseTileTypeGenerationPass {
public:
    BaseTileTypeGenerationPass(const GenerationConfig& config, Xoroshiro128PP rng);
    
    void operator()(TileMap& tilemap);
    void generate_chunk(TileMap& tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y);
    void generate_subchunk(
        TileMap& tilemap, std::uint8_t chunk_x, std::uint8_t chunk_y,
        SubChunkPos sub_pos, BiomeType biome
    );

private:
    BaseTileType determine_base_type(
        double noise_value, const BiomeProperties& properties
    ) const;
};
```

**Key Features:**
- **Biome-aware Generation**: Uses biome properties to control terrain type ratios
- **Hierarchical Processing**: Processes chunks, then sub-chunks, then individual tiles
- **Noise-based Distribution**: Uses calibrated noise for balanced terrain distribution
- **Tile-level Detail**: Generates terrain at individual tile resolution

### TerrainGenerator

Main orchestrator class that manages the generation process using multiple passes.

```cpp
class TerrainGenerator {
public:
    explicit TerrainGenerator(const GenerationConfig& config);
    void operator()(TileMap& tilemap);

private:
    void biome_pass(TileMap& tilemap);
    void base_tile_type_pass(TileMap& tilemap);
};
```

**Generation Flow:**
1. **Biome Pass**: Generate climate data and assign biomes to sub-chunks
2. **Base Tile Type Pass**: Generate base terrain types based on biomes and noise

### Generation Function

Convenience function for complete map generation.

```cpp
void map_generate(TileMap& tilemap, const GenerationConfig& config);
```

## Random Number Generation

### Seed

128-bit seed structure for random number generation.

```cpp
struct Seed {
    std::uint64_t s[2];                           // 128-bit seed value (two 64-bit components)

    static Seed from_string(const char* str);     // Create seed from string
    static Seed device_random();                  // Create seed from hardware random device
};
```

**Key Features:**
- **128-bit precision**: Uses two 64-bit integers for extended seed space
- **String generation**: Deterministic seed creation from text strings
- **Hardware random**: True random seed generation using system entropy

### Xoroshiro128++

High-performance random number generator using the Xoroshiro128++ algorithm.

```cpp
class Xoroshiro128PP {
public:
    Xoroshiro128PP() = default;
    Xoroshiro128PP(Seed seed);

    // STL RandomEngine interface
    using result_type = std::uint64_t;
    static constexpr result_type min();
    static constexpr result_type max();
    result_type operator()();

    std::uint64_t next();                         // Generate next random number
    Xoroshiro128PP jump_64() const;               // Jump equivalent to 2^64 calls
    Xoroshiro128PP jump_96() const;               // Jump equivalent to 2^96 calls
};
```

**Key Features:**
- **High Performance**: Optimized for speed with excellent statistical properties
- **128-bit State**: Internal state provides long period (2^128 - 1)
- **Jump Functions**: Enable parallel random number generation
- **STL Compatible**: Implements standard random engine interface

## Noise System

### PerlinNoise

Standard Perlin noise implementation using Xoroshiro128++ for procedural generation.

```cpp
class PerlinNoise {
public:
    explicit PerlinNoise(Xoroshiro128PP rng);
    double noise(double x, double y) const;
    double octave_noise(double x, double y, int octaves = 4, double persistence = 0.5) const;
};
```

### UniformPerlinNoise

Advanced noise generator using Xoroshiro128++ that provides uniform distribution mapping.

```cpp
class UniformPerlinNoise {
public:
    explicit UniformPerlinNoise(Xoroshiro128PP rng);
    void calibrate(double scale, int octaves = 1, double persistence = 0.5, int sample_size = 10000);
    double uniform_noise(double x, double y) const;
    bool is_calibrated() const;
};
```

**Key Features:**
- **Calibration**: Samples noise distribution to build CDF
- **Uniform Mapping**: Maps raw Perlin values to uniform [0,1] distribution
- **Balanced Output**: Ensures even distribution across all value ranges
- **Automatic Use**: TerrainGenerator uses this internally for balanced terrain
- **Xoroshiro128++ Backend**: Uses high-quality random number generation

## Biome System

### BiomeType

Available biome types based on temperature and humidity.

```cpp
enum class BiomeType : std::uint8_t {
    SnowyPeeks,   // Cold & Dry
    SnowyPlains,  // Cold & Moderate
    FrozenOcean,  // Cold & Wet
    Plains,       // Temperate & Dry
    Forest,       // Temperate & Moderate
    Ocean,        // Temperate & Wet
    Desert,       // Hot & Dry
    Savanna,      // Hot & Moderate
    LukeOcean,    // Hot & Wet
};
```

### BiomeProperties

Properties that control terrain generation for each biome.

```cpp
struct BiomeProperties {
    std::string_view name;      // Biome name
    double water_ratio;         // Water generation ratio
    double ice_ratio;           // Ice generation ratio
    double sand_ratio;          // Sand generation ratio
    double land_ratio;          // Land generation ratio
    int base_octaves = 3;       // Noise octaves
    double base_persistence = 0.5; // Noise persistence
};
```

### Biome Functions

```cpp
const BiomeProperties& get_biome_properties(BiomeType biome);
BiomeType determine_biome(double temperature, double humidity);
```

## Usage Examples

### Basic Map Generation

```cpp
#include "tilemap.h"
#include "generation.h"

// Create a 4x4 chunk map
istd::TileMap tilemap(4);

// Configure generation
istd::GenerationConfig config;
config.seed = istd::Seed::from_string("hello_world");  // 128-bit seed from string

// Temperature noise settings
config.temperature_scale = 0.05;
config.temperature_octaves = 3;
config.temperature_persistence = 0.4;

// Humidity noise settings
config.humidity_scale = 0.05;
config.humidity_octaves = 3;
config.humidity_persistence = 0.4;

// Base terrain noise settings
config.base_scale = 0.08;
config.base_octaves = 3;
config.base_persistence = 0.5;

// Generate terrain
istd::map_generate(tilemap, config);

// Access tiles
for (int chunk_y = 0; chunk_y < tilemap.get_size(); ++chunk_y) {
    for (int chunk_x = 0; chunk_x < tilemap.get_size(); ++chunk_x) {
        const auto& chunk = tilemap.get_chunk(chunk_x, chunk_y);
        // Process chunk tiles...
    }
}
```

### Advanced Generation with Custom Passes

```cpp
#include "tilemap.h"
#include "generation.h"

// Create map and config
istd::TileMap tilemap(2);
istd::GenerationConfig config;
config.seed = istd::Seed::from_string("custom_world");

// Use TerrainGenerator for step-by-step control
istd::TerrainGenerator generator(config);

// Generate terrain (runs both biome and base tile passes)
generator(tilemap);

// Access biome data
const auto& chunk = tilemap.get_chunk(0, 0);
for (int sub_y = 0; sub_y < istd::Chunk::subchunk_count; ++sub_y) {
    for (int sub_x = 0; sub_x < istd::Chunk::subchunk_count; ++sub_x) {
        istd::SubChunkPos pos(sub_x, sub_y);
        istd::BiomeType biome = chunk.get_biome(pos);
        const auto& props = istd::get_biome_properties(biome);
        // Process biome...
    }
}
```
```

### Seed Usage Examples

```cpp
// Create seed from string (deterministic)
istd::Seed seed1 = istd::Seed::from_string("my_world");

// Create random seed from hardware
istd::Seed seed2 = istd::Seed::device_random();

// Manual seed creation
istd::Seed seed3;
seed3.s[0] = 0x123456789abcdef0;
seed3.s[1] = 0xfedcba9876543210;

// Use seed in generation
istd::GenerationConfig config;
config.seed = seed1;
```

### Accessing Individual Tiles

```cpp
// Using TilePos
istd::TilePos pos{0, 0, 32, 32}; // Chunk (0,0), tile (32,32)
const auto& tile = tilemap.get_tile(pos);

// Direct chunk access
const auto& chunk = tilemap.get_chunk(0, 0);
const auto& tile2 = chunk.tiles[32][32];
```

### Working with Biomes

```cpp
// Method 1: Direct array access (traditional way)
const auto& chunk = tilemap.get_chunk(0, 0);
istd::BiomeType biome = chunk.biome[1][1]; // Sub-chunk (1,1)

// Method 2: Using SubChunkPos and get_biome method (recommended)
istd::SubChunkPos pos(1, 1); // Sub-chunk (1,1)
istd::BiomeType biome2 = chunk.get_biome(pos);

// Modify biome using the new method
auto& mutable_chunk = tilemap.get_chunk(0, 0);
mutable_chunk.get_biome(pos) = istd::BiomeType::Forest;

// Get biome properties
const auto& props = istd::get_biome_properties(biome);
std::cout << "Biome: " << props.name << std::endl;
```

## Performance Notes

- Each chunk contains 4,096 tiles (64×64)
- Sub-chunks provide efficient biome management
- Tiles are packed into 1 byte each for memory efficiency
- Generation uses Xoroshiro128++ random number generator with uniform distribution mapping for balanced terrain
- Noise calibration is performed once during generator construction
- 128-bit seeds provide excellent randomness and reproducibility

## Noise Distribution

The library uses an advanced noise system based on Xoroshiro128++ random number generation that addresses the non-uniform distribution of Perlin noise:

### Problem with Raw Perlin Noise

Raw Perlin noise follows a bell-curve distribution, with most values concentrated around 0.5. This leads to unbalanced terrain generation where certain tile types (like Land) dominate the map.

### Solution: Xoroshiro128++ + Uniform Distribution Mapping

The library combines two key improvements:

1. **Xoroshiro128++ RNG**: High-quality pseudo-random number generator with:
   - **Long Period**: 2^128 - 1 sequence length before repetition
   - **High Performance**: Optimized for speed and memory efficiency
   - **Excellent Statistics**: Passes rigorous randomness tests
   - **128-bit State**: Two 64-bit values providing extensive seed space

2. **Uniform Distribution Mapping**: The `UniformPerlinNoise` class:
   - **Samples** the noise distribution during calibration
   - **Builds a CDF** (Cumulative Distribution Function) from the samples
   - **Maps raw noise values** to uniform [0,1] distribution using quantiles
   - **Ensures balanced** terrain type distribution according to biome properties

### Usage in Terrain Generation

```cpp
// The terrain generator automatically uses Xoroshiro128++ and uniform noise
istd::Seed seed = istd::Seed::from_string("consistent_world");
istd::GenerationConfig config;
config.seed = seed;

// TerrainGenerator handles pass coordination and RNG management
istd::TerrainGenerator generator(config);
generator(tilemap);  // Uses calibrated uniform noise with Xoroshiro128++

// Or use the convenience function
istd::map_generate(tilemap, config);
```

### Pass-based Architecture Benefits

The new pass-based system provides:

1. **Separation of Concerns**: Each pass handles a specific aspect of generation
2. **RNG Independence**: Each pass uses independent random number generators
3. **Reproducible Results**: Same seed produces identical results across passes
4. **Extensibility**: Easy to add new passes or modify existing ones
5. **Performance**: Efficient memory access patterns and reduced redundant calculations

## Thread Safety

The library is not inherently thread-safe. External synchronization is required for concurrent access to TileMap objects.
