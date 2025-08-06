cmake_minimum_required(VERSION 3.27)

set(ISTD_CORE_SRC
	src/devices/vehicle.cpp
	src/device.cpp
	src/room.cpp
	src/system.cpp
	src/unit.cpp
	src/world.cpp
)

add_library(istd_core STATIC ${ISTD_CORE_SRC})
target_include_directories(istd_core PUBLIC include)
target_link_libraries(istd_core PUBLIC istd_util istd_tilemap EnTT)
target_compile_features(istd_core PUBLIC cxx_std_23)
