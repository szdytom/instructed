cmake_minimum_required(VERSION 3.27)
project(instructed LANGUAGES CXX)

include(CTest)
enable_testing()

# Optionally build examples & Tests
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)

# Load third-party libraries
add_subdirectory(third_party)

# Add util library
add_subdirectory(util)

# Add tilemap library and examples
add_subdirectory(tilemap)
