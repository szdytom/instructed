# Tilemap Library API

## Overview

The tilemap library provides a system for generating and managing tile-based terrain with biome support.

## Core Classes

### TileMap

Main container for the map, organized as chunks.

```cpp
class TileMap {
public:
    explicit TileMap(std::uint8_t size);

    std::uint8_t get_size() const;
    Chunk& get_chunk(std::uint8_t chunk_x, std::uint8_t chunk_y);
    
    Tile& get_tile(TilePos pos);
    const Tile& get_tile(TilePos pos) const;
    void set_tile(TilePos pos, const Tile& tile);
};
```

### Chunk

64×64 tile container with biome information.

```cpp
struct Chunk {
    static constexpr uint8_t size = 64;
    
    Tile tiles[size][size];
    BiomeType biome[16][16];  // Sub-chunk biomes
    
    BiomeType& get_biome(SubChunkPos pos);
};
```

### Tile

Individual map tile with terrain types.

```cpp
struct Tile {
    BaseTileType base : 4;      // Base terrain
    SurfaceTileType surface : 4; // Surface features
};
```

**Base Tile Types:**
- `Land`, `Mountain`, `Sand`, `Water`, `Ice`, `Deepwater`

**Surface Tile Types:**
- `Empty`, `Oil`

### Position Types

```cpp
struct TilePos {
    uint8_t chunk_x, chunk_y;  // Chunk coordinates
    uint8_t local_x, local_y;  // Tile within chunk (0-63)
    
    std::pair<std::uint16_t, std::uint16_t> to_global() const;
    static TilePos from_global(std::uint16_t global_x, std::uint16_t global_y);
};

struct SubChunkPos {
    std::uint8_t sub_x, sub_y;
};
```

## Terrain Generation

### GenerationConfig

Configuration for terrain generation.

```cpp
struct GenerationConfig {
    Seed seed;
    
    // Noise parameters
    double temperature_scale = 0.05;
    double humidity_scale = 0.05;
    double base_scale = 0.08;
    
    int temperature_octaves = 3;
    int humidity_octaves = 3;
    int base_octaves = 3;
    
    // Oil generation parameters
    double oil_density = 0.8;           // Average oil fields per chunk
    std::uint32_t oil_cluster_min_size = 2; // Minimum tiles per cluster
    std::uint32_t oil_cluster_max_size = 6; // Maximum tiles per cluster
    double oil_biome_preference = 2.0;  // Multiplier for preferred biomes
};
```

### Generation Function

```cpp
void map_generate(TileMap& tilemap, const GenerationConfig& config);
```

### Seed

```cpp
struct Seed {
    std::uint64_t s[2];
    
    static Seed from_string(const char* str);
    static Seed device_random();
};
```

## Biomes

### BiomeType

```cpp
enum class BiomeType : std::uint8_t {
    SnowyPeaks, SnowyPlains, FrozenOcean,
    Plains, Forest, Ocean,
    Desert, Savanna, LukeOcean
};
```

### Biome Functions

```cpp
const BiomeProperties& get_biome_properties(BiomeType biome);
BiomeType determine_biome(double temperature, double humidity);
```

## Usage Examples

### Basic Usage

```cpp
#include "tilemap.h"
#include "generation.h"

// Create map
istd::TileMap tilemap(4);  // 4x4 chunks

// Generate terrain
istd::GenerationConfig config;
config.seed = istd::Seed::from_string("my_world");
istd::map_generate(tilemap, config);

// Access tiles
istd::TilePos pos{0, 0, 32, 32};  // Chunk (0,0), tile (32,32)
const auto& tile = tilemap.get_tile(pos);
```

### Working with Biomes

```cpp
const auto& chunk = tilemap.get_chunk(0, 0);
istd::SubChunkPos sub_pos(1, 1);
istd::BiomeType biome = chunk.get_biome(sub_pos);

const auto& props = istd::get_biome_properties(biome);
std::cout << "Biome: " << props.name << std::endl;
```
