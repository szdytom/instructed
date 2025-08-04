cmake_minimum_required(VERSION 3.27)
cmake_policy(VERSION 3.27)
include(FetchContent)

# EnTT third-party library setup
message(STATUS "Downloading EnTT...")
FetchContent_Declare(
	EnTT
	URL "https://github.com/skypjack/entt/archive/refs/tags/v3.15.0.zip"
)
FetchContent_MakeAvailable(EnTT)
message(STATUS "EnTT ready")
