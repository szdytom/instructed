
cmake_minimum_required(VERSION 3.27)

# Define util library sources
add_library(istd_util STATIC
	src/vec2.cpp
)
target_include_directories(istd_util PUBLIC include)
target_compile_features(istd_util PUBLIC cxx_std_23)

if (BUILD_TESTS)
	add_subdirectory("test/")
endif()
