cmake_minimum_required(VERSION 3.27)
project(instructed LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add tilemap library and examples
add_subdirectory(tilemap)
