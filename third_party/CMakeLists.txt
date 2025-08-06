cmake_minimum_required(VERSION 3.27)

set(ISTD_THIRD_PARTY_LIBS
    entt
    asio
	catch2
)

foreach(lib IN LISTS ISTD_THIRD_PARTY_LIBS)
    include(${CMAKE_CURRENT_LIST_DIR}/${lib}.cmake)
endforeach()

