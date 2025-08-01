# Tilemap Library

一个基于Perlin噪声和生物群系系统的C++地图生成库。

## 项目结构

```
tilemap/
├── include/           # 库头文件
│   ├── biome.h       # 生物群系系统
│   ├── chunk.h       # 区块和瓦片定义
│   ├── generation.h  # 地形生成器
│   ├── noise.h       # Perlin噪声实现
│   ├── tile.h        # 瓦片类型定义
│   └── tilemap.h     # 地图容器类
├── src/              # 库源文件
│   ├── biome.cpp     # 生物群系实现
│   ├── generation.cpp # 地形生成实现
│   ├── noise.cpp     # Perlin噪声实现
│   └── tilemap.cpp   # 地图容器实现
├── examples/         # 示例程序
│   ├── basic_demo.cpp          # 基础功能演示
│   ├── advanced_demo.cpp       # 高级功能演示
│   ├── biome_demo.cpp          # 生物群系演示
│   ├── advanced_biome_demo.cpp # 高级生物群系分析
│   └── CMakeLists.txt          # 示例程序构建配置
└── CMakeLists.txt    # 主构建配置
```

## 构建

### 构建库和示例程序

```bash
mkdir build
cd build
cmake ..
make
```

### 仅构建库（不构建示例）

```bash
mkdir build
cd build
cmake -DBUILD_EXAMPLES=OFF ..
make
```

## 运行示例

构建完成后，可执行文件位于 `build/tilemap/examples/` 目录下：

```bash
# 基础演示
./build/tilemap/examples/basic_demo

# 高级功能演示
./build/tilemap/examples/advanced_demo

# 生物群系系统演示
./build/tilemap/examples/biome_demo

# 高级生物群系分析
./build/tilemap/examples/advanced_biome_demo
```

## 库使用

### 基本用法

```cpp
#include "tilemap.h"
#include "generation.h"

// 创建一个10x10区块的地图
istd::TileMap tilemap(10);

// 配置生成参数
istd::GenerationConfig config;
config.seed = 42;

// 生成地图
istd::map_generate(tilemap, config);
```

### 使用生物群系系统

```cpp
#include "generation.h"

istd::GenerationConfig config;
config.seed = 12345;
config.temperature_scale = 0.005;  // 温度变化尺度
config.humidity_scale = 0.007;     // 湿度变化尺度

istd::TerrainGenerator generator(config);
generator.generate_map(tilemap);
```

## 核心特性

- **Perlin噪声地形生成**: 生成自然的地形特征
- **生物群系系统**: 基于温度和湿度的9种生物群系
- **区块系统**: 支持大型地图的高效存储
- **子区块分级**: 16x16瓦片的子区块，每个具有一致的生物群系
- **高性能**: 优化的内存使用和生成算法

## 编译要求

- C++23标准支持
- CMake 3.27或更高版本
- 支持C++23的编译器（GCC 13+, Clang 16+, MSVC 2022+）
