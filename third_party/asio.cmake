cmake_minimum_required(VERSION 3.27)
cmake_policy(VERSION 3.27)
include(FetchContent)

message(STATUS "Downloading Asio(Standalone)...")
# Asio (Standalone) third-party library setup
FetchContent_Declare(
	asio
	URL "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-34-2.zip"
)
FetchContent_MakeAvailable(asio)
message(STATUS "Asio ready")
