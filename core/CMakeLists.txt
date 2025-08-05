cmake_minimum_required(VERSION 3.27)

set(ISTD_CORE_SRC
	src/room.cpp
)

add_library(istd_core STATIC ${ISTD_CORE_SRC})
target_include_directories(istd_core PUBLIC include)
target_link_libraries(istd_core PUBLIC istd_util istd_tilemap EnTT)
target_compile_features(istd_core PUBLIC cxx_std_23)
