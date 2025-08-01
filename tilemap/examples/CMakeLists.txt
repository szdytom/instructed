cmake_minimum_required(VERSION 3.27)

# Examples for the tilemap library
# Each example is built as a separate executable

# Basic demonstration of tilemap generation
add_executable(basic_demo basic_demo.cpp)
target_link_libraries(basic_demo PRIVATE istd_tilemap)
target_include_directories(basic_demo PRIVATE ../include)

# Advanced tilemap features demonstration
add_executable(advanced_demo advanced_demo.cpp)
target_link_libraries(advanced_demo PRIVATE istd_tilemap)
target_include_directories(advanced_demo PRIVATE ../include)

# Basic biome system demonstration
add_executable(biome_demo biome_demo.cpp)
target_link_libraries(biome_demo PRIVATE istd_tilemap)
target_include_directories(biome_demo PRIVATE ../include)

# Advanced biome analysis and visualization
add_executable(advanced_biome_demo advanced_biome_demo.cpp)
target_link_libraries(advanced_biome_demo PRIVATE istd_tilemap)
target_include_directories(advanced_biome_demo PRIVATE ../include)
