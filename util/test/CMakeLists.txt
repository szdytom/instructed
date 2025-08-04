cmake_minimum_required(VERSION 3.27)


add_executable(test_small_map small_map.cpp)
target_link_libraries(test_small_map PRIVATE istd_util)
target_compile_features(test_small_map PRIVATE cxx_std_23)

add_executable(test_vec2 test_vec2.cpp)
target_link_libraries(test_vec2 PRIVATE istd_util)
target_compile_features(test_vec2 PRIVATE cxx_std_23)


include(CTest)
enable_testing()
add_test(NAME util_small_map COMMAND test_small_map)
add_test(NAME util_vec2 COMMAND test_vec2)
