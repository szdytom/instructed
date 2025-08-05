cmake_minimum_required(VERSION 3.27)

include(CTest)
enable_testing()

function(declare_istd_util_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE istd_util)
    target_compile_features(${name} PRIVATE cxx_std_23)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

declare_istd_util_test(test_small_map small_map.cpp)
declare_istd_util_test(test_vec2 test_vec2.cpp)
declare_istd_util_test(test_tile_geometry test_tile_geometry.cpp)
