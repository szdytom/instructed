cmake_minimum_required(VERSION 3.27)

# Examples for the tilemap library
# Each example is built as a separate executable

# Dual-noise terrain generation demonstration
add_executable(dual_noise_demo dual_noise_demo.cpp)
target_link_libraries(dual_noise_demo PRIVATE istd_tilemap)
target_include_directories(dual_noise_demo PRIVATE ../include)

# Biome system demonstration
add_executable(biome_demo biome_demo.cpp)
target_link_libraries(biome_demo PRIVATE istd_tilemap)
target_include_directories(biome_demo PRIVATE ../include)
