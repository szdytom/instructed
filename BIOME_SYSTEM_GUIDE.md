# 生物群系系统使用指南

## 概述

新的生物群系系统基于温度和湿度参数来决定地形生成，提供了更加真实和多样化的地图生成体验。

## 核心特性

### 1. 生物群系类型
系统支持9种不同的生物群系，基于温度（冷/温带/热）和湿度（干燥/适中/潮湿）的组合：

| 温度\湿度 | 干燥 | 适中 | 潮湿 |
|----------|------|------|------|
| **冷** | 苔原(Tundra) | 针叶林(Taiga) | 寒带雨林(ColdRainforest) |
| **温带** | 草原(Grassland) | 落叶林(DeciduousForest) | 温带雨林(TemperateRainforest) |
| **热** | 沙漠(Desert) | 热带草原(Savanna) | 热带雨林(TropicalRainforest) |

### 2. 子区块系统
- 每个64×64的区块被划分为4×4个子区块
- 每个子区块大小为16×16瓦片
- 每个子区块具有一致的生物群系
- 生物群系基于子区块中心位置的气候值确定

### 3. 地形生成参数
每个生物群系都有独特的地形生成参数：
- **水域阈值**: 决定水体分布
- **沙地阈值**: 决定沙地区域
- **森林阈值**: 决定树木覆盖
- **山地阈值**: 决定山脉分布
- **噪声参数**: 控制地形的细节程度

## 使用方法

### 基本使用

```cpp
#include "generation.h"
#include "tilemap.h"

// 创建地图
istd::TileMap tilemap(10);

// 配置生成参数
istd::GenerationConfig config;
config.seed = 12345;
config.temperature_scale = 0.005; // 温度变化尺度
config.humidity_scale = 0.007;    // 湿度变化尺度

// 生成地图
istd::map_generate(tilemap, config);
```

### 使用TerrainGenerator类

```cpp
// 创建生成器实例
istd::TerrainGenerator generator(config);

// 生成地图
generator.generate_map(tilemap);
// 生物群系数据会在生成完成后自动清理
```

### 获取生物群系信息

```cpp
// 获取特定位置的气候值
double global_x = chunk_x * 64 + sub_x * 16 + 8;
double global_y = chunk_y * 64 + sub_y * 16 + 8;

// 使用噪声获取气候
istd::PerlinNoise temp_noise(seed + 1000);
istd::PerlinNoise humidity_noise(seed + 2000);

double temperature = temp_noise.octave_noise(
    global_x * temperature_scale, global_y * temperature_scale, 3, 0.5);
double humidity = humidity_noise.octave_noise(
    global_x * humidity_scale, global_y * humidity_scale, 3, 0.5);

// 确定生物群系
istd::BiomeType biome = istd::determine_biome(temperature, humidity);

// 获取生物群系属性
const istd::BiomeProperties& props = istd::get_biome_properties(biome);
```

## 坐标系统

### SubChunkPos结构
```cpp
istd::SubChunkPos sub_pos(2, 1); // 子区块(2,1)
```

### 坐标转换
```cpp
// 瓦片坐标转子区块坐标
istd::SubChunkPos sub_pos = istd::tile_to_subchunk(local_x, local_y);

// 子区块坐标转瓦片起始坐标
auto [start_x, start_y] = istd::subchunk_to_tile_start(sub_pos);
```

## 配置参数详解

### GenerationConfig参数

- **seed**: 随机种子，控制整体地图布局
- **temperature_scale**: 温度噪声的缩放因子
  - 较小值 (0.001-0.005): 大规模气候区域
  - 较大值 (0.01-0.02): 小规模气候变化
- **humidity_scale**: 湿度噪声的缩放因子
  - 建议比temperature_scale稍大，产生不同的气候模式

### 生物群系属性示例

```cpp
// 沙漠生物群系
{
    .water_threshold = 0.1,        // 很少水体
    .sand_threshold = 0.7,         // 大量沙地
    .wood_threshold = 0.85,        // 很少植被
    .mountain_threshold = 0.9,     // 适中山地
    .scale = 0.03,                 // 地形噪声尺度
    .octaves = 3,                  // 噪声层数
    .persistence = 0.4,            // 噪声持续性
    .name = "Desert"
}
```

## 最佳实践

### 1. 气候尺度设置
- 对于大陆级地图: `temperature_scale = 0.001-0.003`
- 对于区域级地图: `temperature_scale = 0.005-0.01`
- 对于局部地图: `temperature_scale = 0.01-0.02`

### 2. 生物群系多样性
- 使用不同的种子可以产生完全不同的气候分布
- 调整温度和湿度尺度的比例可以改变生物群系的形状和分布

### 3. 性能考虑
- 生物群系数据仅在生成过程中存储，生成完成后自动释放
- 大地图建议分块生成以控制内存使用

## 演示程序

- `biome_demo`: 基础生物群系演示
- `advanced_biome_demo`: 高级生物群系分析和可视化
- `tilemap_demo`: 传统兼容模式演示

运行示例：
```bash
./build/tilemap/biome_demo
./build/tilemap/advanced_biome_demo
```
