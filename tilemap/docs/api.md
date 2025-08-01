# Tilemap Library API Documentation

## Overview

The tilemap library provides a flexible system for generating and managing tile-based terrain with biome support. The library consists of several main components:

- **TileMap**: The main map container holding chunks of tiles
- **Chunk**: 64x64 tile containers with biome information  
- **Tile**: Individual map tiles with base and surface types
- **TerrainGenerator**: Procedural terrain generation system
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
    
    Tile& get_tile(const TilePos& pos);
    const Tile& get_tile(const TilePos& pos) const;
    void set_tile(const TilePos& pos, const Tile& tile);
};
```

**Constructor Parameters:**
- `size`: Number of chunks per side (max 100), creating an n×n grid

### Chunk

Each chunk contains 64×64 tiles and sub-chunk biome information.

```cpp
struct Chunk {
    static constexpr uint8_t size = 64;           // Tiles per side
    static constexpr uint8_t subchunk_size = /* certain value */;  // Tiles per sub-chunk side
    static constexpr uint8_t subchunk_count = size / subchunk_size;  // Sub-chunks per side
    
    Tile tiles[size][size];                       // 64x64 tile grid
    BiomeType biome[subchunk_count][subchunk_count];
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

## Terrain Generation

### GenerationConfig

Configuration parameters for terrain generation.

```cpp
struct GenerationConfig {
    std::uint64_t seed = 0;                    // Random seed
    double temperature_scale = 0.005;          // Temperature noise scale
    double humidity_scale = 0.007;             // Humidity noise scale  
    double base_scale = 0.08;                  // Base terrain noise scale
};
```

### TerrainGenerator

Main class for procedural terrain generation.

```cpp
class TerrainGenerator {
public:
    explicit TerrainGenerator(const GenerationConfig& config);
    void generate_map(TileMap& tilemap);
};
```

### Generation Function

Convenience function for map generation.

```cpp
void map_generate(TileMap& tilemap, const GenerationConfig& config);
```

## Noise System

### PerlinNoise

Standard Perlin noise implementation for procedural generation.

```cpp
class PerlinNoise {
public:
    explicit PerlinNoise(std::uint64_t seed = 0);
    double noise(double x, double y) const;
    double octave_noise(double x, double y, int octaves = 4, double persistence = 0.5) const;
};
```

### UniformPerlinNoise

Advanced noise generator that provides uniform distribution mapping.

```cpp
class UniformPerlinNoise {
public:
    explicit UniformPerlinNoise(std::uint64_t seed = 0);
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

### SubChunkPos

Position within a chunk's sub-chunk grid.

```cpp
struct SubChunkPos {
    std::uint8_t sub_x;
    std::uint8_t sub_y;
};

constexpr std::pair<std::uint8_t, std::uint8_t> subchunk_to_tile_start(
    const SubChunkPos& pos
);
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
config.seed = 12345;
config.temperature_scale = 0.005;
config.humidity_scale = 0.007;
config.base_scale = 0.08;

// Generate terrain
istd::map_generate(tilemap, config);

// Access tiles
for (int chunk_y = 0; chunk_y < Chunk::subchunk_count; ++chunk_y) {
    for (int chunk_x = 0; chunk_x < Chunk::subchunk_count; ++chunk_x) {
        const auto& chunk = tilemap.get_chunk(chunk_x, chunk_y);
        // Process chunk tiles...
    }
}
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
// Get biome for a sub-chunk
const auto& chunk = tilemap.get_chunk(0, 0);
istd::BiomeType biome = chunk.biome[1][1]; // Sub-chunk (1,1)

// Get biome properties
const auto& props = istd::get_biome_properties(biome);
std::cout << "Biome: " << props.name << std::endl;
```

## Performance Notes

- Each chunk contains 4,096 tiles (64×64)
- Sub-chunks provide efficient biome management
- Tiles are packed into 1 byte each for memory efficiency
- Generation uses Perlin noise with uniform distribution mapping for balanced terrain
- Noise calibration is performed once during generator construction

## Noise Distribution

The library uses an advanced noise system that addresses the non-uniform distribution of Perlin noise:

### Problem with Raw Perlin Noise

Raw Perlin noise follows a bell-curve distribution, with most values concentrated around 0.5. This leads to unbalanced terrain generation where certain tile types (like Land) dominate the map.

### Solution: Uniform Distribution Mapping

The `UniformPerlinNoise` class:

1. **Samples** the noise distribution during calibration
2. **Builds a CDF** (Cumulative Distribution Function) from the samples
3. **Maps raw noise values** to uniform [0,1] distribution using quantiles
4. **Ensures balanced** terrain type distribution according to biome properties

### Usage in Terrain Generation

```cpp
// The terrain generator automatically uses uniform noise
TerrainGenerator generator(config);
generator.generate_map(tilemap);  // Uses calibrated uniform noise internally
```

## Thread Safety

The library is not inherently thread-safe. External synchronization is required for concurrent access to TileMap objects.
