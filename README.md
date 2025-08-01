# Instructed Project

一个使用现代C++实现的地图生成系统项目。

## 项目组成

### 🗺️ Tilemap Library
位于 `tilemap/` 目录下的核心地图生成库，提供：

- **Perlin噪声地形生成**: 基于噪声算法的自然地形
- **生物群系系统**: 9种基于气候的生物群系
- **高效区块系统**: 支持大规模地图生成
- **现代C++设计**: 使用C++23标准

### 📁 项目结构

```
instructed/
├── tilemap/                    # Tilemap库
│   ├── include/               # 头文件
│   ├── src/                   # 库源代码
│   ├── examples/              # 示例程序
│   └── README.md              # 库文档
├── CMakeLists.txt             # 主构建文件
└── README.md                  # 项目说明
```

## 🚀 快速开始

### 构建项目

```bash
# 克隆或下载项目
cd instructed

# 构建
mkdir build && cd build
cmake ..
make

# 运行示例
./build/tilemap/examples/basic_demo
```

### 禁用示例程序构建

```bash
cmake -DBUILD_EXAMPLES=OFF ..
make
```

## 🎮 示例程序

| 程序 | 描述 |
|------|------|
| `basic_demo` | 基础地图生成演示 |
| `advanced_demo` | 高级功能和统计信息 |
| `biome_demo` | 生物群系系统演示 |
| `advanced_biome_demo` | 生物群系分析和可视化 |

## 📖 文档

- [Tilemap库使用指南](tilemap/README.md)
- [生物群系系统详解](BIOME_SYSTEM_GUIDE.md)
- [传统使用方法](TILEMAP_USAGE.md)

## 🛠️ 技术要求

- **C++23**: 现代C++特性支持
- **CMake 3.27+**: 构建系统
- **支持的编译器**: GCC 13+, Clang 16+, MSVC 2022+

## 📝 开发说明

这个项目展示了：
- 清晰的库和示例分离
- 现代CMake最佳实践
- 模块化的C++库设计
- 完整的文档和示例

## Features

- **Chunk-based Architecture**: Divides the world into 64x64 tile chunks for efficient memory management
- **Multiple Tile Types**: Supports different terrain types (Empty, Grass, Stone, Water, Sand, Forest)
- **Perlin Noise Generation**: Uses Perlin noise algorithm for natural-looking terrain generation
- **Flexible Map Size**: Support for n×n chunks (configurable map dimensions)
- **Efficient Access**: Fast tile and chunk access with coordinate conversion utilities
- **Tile Properties**: Tiles have properties like walkability and liquid state

## Project Structure

```
include/
├── tile.h      # Individual tile class
├── chunk.h     # 64x64 chunk of tiles
├── tilemap.h   # Main tilemap manager
└── random.h    # Random number and noise generation

src/
├── tile.cpp
├── chunk.cpp
├── tilemap.cpp
├── random.cpp
└── main.cpp    # Demo application
```

## Building

### Option 1: Using Make
```bash
make
```

### Option 2: Using CMake
```bash
make cmake
```

### Option 3: Manual compilation
```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src/*.cpp -o tilemap_demo
```

## Running

```bash
# Using make
make run

# Or directly
./build/tilemap_demo
```

## Usage Example

```cpp
#include "tilemap.h"

int main() {
    // Create a 4x4 tilemap (256x256 tiles total)
    TileMap tileMap(4, 4);
    
    // Generate terrain using Perlin noise
    tileMap.generatePerlin(54321, 0.05f);
    
    // Access individual tiles
    Tile& tile = tileMap.getTile(100, 100);
    tile.setType(Tile::WATER);
    
    // Access chunks
    Chunk* chunk = tileMap.getChunk(1, 1);
    if (chunk) {
        chunk->setTile(32, 32, Tile(Tile::STONE));
    }
    
    // Print map overview
    tileMap.printMap();
    
    return 0;
}
```

## Key Classes

### Tile
- Represents a single tile with a type (Empty, Grass, Stone, Water, Sand, Forest)
- Provides utility methods like `isWalkable()` and `isLiquid()`

### Chunk
- Contains a 64x64 array of tiles
- Supports Perlin noise generation for natural terrain
- Manages local tile coordinates within the chunk

### TileMap
- Manages multiple chunks to form a complete world
- Handles coordinate conversion between world and chunk coordinates
- Provides unified access to tiles across chunk boundaries

### Random
- Utility class for random number generation  
- Implements Perlin noise for natural terrain generation
- Supports seeded generation for reproducible results

## Coordinate System

- **World Coordinates**: Global tile positions (0,0) to (worldWidth-1, worldHeight-1)
- **Chunk Coordinates**: Chunk positions (0,0) to (mapWidth-1, mapHeight-1)
- **Local Coordinates**: Tile positions within a chunk (0,0) to (63,63)

The system automatically converts between coordinate systems as needed.

## Customization

You can easily extend the system by:
- Adding new tile types to the `Tile::Type` enum
- Implementing custom generation algorithms in `Chunk`
- Modifying tile properties and behaviors
- Adding new terrain features or biomes

## Performance Notes

- Each chunk contains 4,096 tiles (64×64)
- Memory usage scales with the number of active chunks
- Coordinate conversion is O(1)
- Tile access within a chunk is O(1)
