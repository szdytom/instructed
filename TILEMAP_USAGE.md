# Perlin Noise Tilemap Generator

这个项目实现了一个基于Perlin噪声的C++地图生成系统，可以生成包含不同地形类型的n×n区块地图。

## 功能特性

- **Perlin噪声生成**: 使用高质量的Perlin噪声算法生成自然的地形
- **区块系统**: 支持最大100×100区块的大型地图
- **TilePos坐标系统**: 使用chunk_x, chunk_y, local_x, local_y的四元坐标系统
- **多种地形类型**: 支持空地、山地、森林、沙地、水域五种地形
- **可配置生成**: 提供种子、缩放、八度等可调参数

## 使用示例

```cpp
#include "generation.h"
#include "tilemap.h"

int main() {
    // 创建一个10×10区块的地图
    istd::TileMap tilemap(10);
    
    // 配置生成参数
    istd::GenerationConfig config;
    config.seed = 12345;
    config.scale = 0.02;
    config.octaves = 4;
    config.persistence = 0.5;
    
    // 生成地图
    istd::map_generate(tilemap, config);
    
    // 访问特定位置的瓦片
    istd::TilePos pos{5, 5, 32, 32}; // 区块(5,5)的本地坐标(32,32)
    istd::Tile tile = tilemap.get_tile(pos);
    
    // 修改瓦片
    istd::Tile mountain = istd::Tile::from_name("mountain");
    tilemap.set_tile(pos, mountain);
    
    return 0;
}
```

## 地形类型

- **空地** (`empty`): 类型0，显示为空格' '
- **山地** (`mountain`): 类型1，显示为'^'
- **森林** (`wood`): 类型2，显示为'T'
- **沙地** (`sand`): 类型3，显示为'.'
- **水域** (`water`): 类型4，显示为'~'

## 坐标系统

使用`TilePos`结构体表示地图中的位置：
- `chunk_x`, `chunk_y`: 区块坐标 (0-99)
- `local_x`, `local_y`: 区块内本地坐标 (0-63)

每个区块包含64×64个瓦片。

## 构建和运行

```bash
# 构建项目
cmake -S . -B build && cmake --build build

# 运行基础演示
./build/tilemap/tilemap_demo

# 运行高级演示
./build/tilemap/tilemap_advanced_demo
```

## API参考

### TileMap类
- `TileMap(uint8_t size)`: 构造指定大小的地图
- `get_tile(const TilePos& pos)`: 获取指定位置的瓦片
- `set_tile(const TilePos& pos, const Tile& tile)`: 设置指定位置的瓦片
- `get_chunk(uint8_t x, uint8_t y)`: 获取指定区块

### GenerationConfig结构体
- `seed`: 随机种子
- `scale`: 噪声坐标缩放
- `octaves`: 噪声八度数量
- `persistence`: 八度持续性
- `*_threshold`: 各地形类型的噪声阈值

### PerlinNoise类
- `PerlinNoise(uint64_t seed)`: 构造噪声生成器
- `noise(double x, double y)`: 生成2D噪声值
- `octave_noise(...)`: 生成多八度噪声值
