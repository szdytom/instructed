cmake_minimum_required(VERSION 3.27)

include(CTest)
enable_testing()

# Create a unified test executable from multiple source files
add_executable(istd_util_tests 
    test_small_map.cpp
    test_vec2.cpp 
    test_tile_geometry.cpp
)

target_link_libraries(istd_util_tests PRIVATE istd_util Catch2::Catch2WithMain)
target_compile_features(istd_util_tests PRIVATE cxx_std_23)

# Add the test to CTest
add_test(NAME istd_util_tests COMMAND istd_util_tests)
