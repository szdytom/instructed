cmake_minimum_required(VERSION 3.27)

# Define the tilemap library source files
set(ISTD_TILEMAP_SRC
	src/generation.cpp
	src/tilemap.cpp
	src/noise.cpp
	src/biome.cpp
)

# Create the tilemap library
add_library(istd_tilemap ${ISTD_TILEMAP_SRC})

# Set library properties
target_compile_features(istd_tilemap PUBLIC cxx_std_23)
target_include_directories(istd_tilemap PUBLIC include)

# Optionally build examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
