cmake_minimum_required(VERSION 3.27)
cmake_policy(VERSION 3.27)
include(FetchContent)

# Catch2 third-party library setup
message(STATUS "Downloading Catch2...")
FetchContent_Declare(
	Catch2
	URL "https://github.com/catchorg/Catch2/archive/refs/tags/v3.9.0.zip"
)
FetchContent_MakeAvailable(Catch2)
message(STATUS "Catch2 ready")
