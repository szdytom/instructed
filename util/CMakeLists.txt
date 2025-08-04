cmake_minimum_required(VERSION 3.27)

# Define util library sources (header-only for now)
add_library(istd_util INTERFACE)
target_include_directories(istd_util INTERFACE include)
target_compile_features(istd_util INTERFACE cxx_std_23)

if (BUILD_TESTS)
	add_subdirectory("test/")
endif()
