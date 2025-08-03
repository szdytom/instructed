cmake_minimum_required(VERSION 3.27)

# Examples for the tilemap library
# Each example is built as a separate executable

# Biome system demonstration
add_executable(tilemap_demo tilemap_demo.cpp)
target_link_libraries(tilemap_demo PRIVATE istd_tilemap)
target_include_directories(tilemap_demo PRIVATE ../include)
