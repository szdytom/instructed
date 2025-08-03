# 矿物生成系统实现总结

## 概述

为tilemap库实现了三种新矿石（Hematite赤铁矿、Titanomagnetite钛磁铁矿、Gibbsite三水铝石）的生成系统。该系统基于Oil生成方式的改进版本，专门针对山脉边缘的矿物分布进行了优化。

## 设计选择

### 为什么选择基于Oil的方案而不是胞元自动机？

1. **精确控制性**：Poisson disk采样方式能够精确控制矿物密度和分布间距
2. **效率优势**：单次随机游走比多轮胞元自动机迭代更高效
3. **参数直观性**：密度、集群大小、最小距离等参数更易于调整和平衡
4. **稀有资源特性**：矿物作为稀有资源，稀疏分布更符合游戏设计需求

## 核心特性

### 1. 位置限制
- 矿物只在 `BaseTileType::Mountain` 且 `SurfaceTileType::Empty` 的瓦片上生成
- 必须位于山脉边缘（至少有一个相邻瓦片不是山地）
- 确保矿物出现在山脉与其他地形的交界处，便于开采

### 2. 分层稀有度
```cpp
// 默认配置
hematite_density = 51;        // ~0.2 per chunk (最常见)
titanomagnetite_density = 25; // ~0.1 per chunk (中等稀有)
gibbsite_density = 13;        // ~0.05 per chunk (最稀有)
```

### 3. 集群生成
- 最小集群大小：2个瓦片
- 最大集群大小：5个瓦片
- 使用随机游走算法形成自然的小簇分布
- 40%概率跳过相邻瓦片，形成合适的密度

### 4. 距离控制
- 基于密度动态计算最小间距
- 确保矿物集群不会过于密集
- 最小间距至少8个瓦片

## 实现细节

### 核心算法
1. **Poisson disk采样**：生成候选位置，确保合适的分布
2. **山脉边缘检测**：验证位置是否在山脉边缘
3. **随机游走集群生长**：从中心点开始生成小簇
4. **冲突避免**：确保不同矿物集群之间保持距离

### 山脉边缘检测逻辑
```cpp
bool is_mountain_edge(const TileMap &tilemap, TilePos pos) const {
    auto neighbors = tilemap.get_neighbors(pos);
    for (const auto neighbor_pos : neighbors) {
        const Tile &neighbor_tile = tilemap.get_tile(neighbor_pos);
        if (neighbor_tile.base != BaseTileType::Mountain) {
            return true; // 找到非山地邻居
        }
    }
    return false; // 所有邻居都是山地，不是边缘
}
```

### 配置参数
```cpp
struct GenerationConfig {
    // 矿物集群生成参数
    std::uint8_t hematite_density = 51;        // ~0.2 per chunk
    std::uint8_t titanomagnetite_density = 25; // ~0.1 per chunk  
    std::uint8_t gibbsite_density = 13;        // ~0.05 per chunk
    
    std::uint8_t mineral_cluster_min_size = 2; // 最小集群大小
    std::uint8_t mineral_cluster_max_size = 5; // 最大集群大小
    std::uint8_t mineral_base_probe = 192;     // 基础放置概率
};
```

## 生成流水线集成

矿物生成作为独立的pass添加到地形生成流水线的最后阶段：

```cpp
void TerrainGenerator::operator()(TileMap &tilemap) {
    biome_pass(tilemap);
    base_tile_type_pass(tilemap);
    smoothen_mountains_pass(tilemap);
    smoothen_islands_pass(tilemap);
    mountain_hole_fill_pass(tilemap);
    deepwater_pass(tilemap);
    oil_pass(tilemap);
    mineral_cluster_pass(tilemap);  // 新增的矿物生成pass
}
```

## 测试结果

通过mineral_demo测试程序验证：
- 8x8 chunk地图 (262,144个瓦片)
- 山地瓦片：35,916个 (13.7%)
- 山脉边缘瓦片：15,881个 (44.2%的山地)
- 生成矿物分布：
  - 赤铁矿：47个瓦片
  - 钛磁铁矿：38个瓦片  
  - 三水铝石：15个瓦片
- 山脉边缘矿物覆盖率：0.63%

## 优势

1. **游戏平衡性**：稀有度分层，符合游戏经济设计
2. **真实感**：矿物出现在山脉边缘，符合地质常识
3. **可扩展性**：易于添加新的矿物类型和调整参数
4. **性能优秀**：单次生成，不需要多轮迭代
5. **确定性**：相同种子产生相同结果，支持多人游戏

## 使用建议

1. **密度调整**：根据游戏需求调整各矿物的density参数
2. **集群大小**：可以为不同矿物设置不同的集群大小范围
3. **生成位置**：如需其他位置生成矿物，可修改`is_suitable_for_mineral`函数
4. **稀有度平衡**：建议保持gibbsite < titanomagnetite < hematite的稀有度关系

这个实现提供了灵活、高效且平衡的矿物生成系统，完全满足了"控制生成数量，以小簇方式生成在山的边缘"的需求。
