cmake_minimum_required(VERSION 3.27)
project(instructed LANGUAGES CXX)

# Optionally build examples
option(BUILD_EXAMPLES "Build example programs" ON)

# Load third-party libraries
add_subdirectory(third_party)

# Add tilemap library and examples
add_subdirectory(tilemap)
