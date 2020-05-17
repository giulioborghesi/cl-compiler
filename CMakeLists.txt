cmake_minimum_required(VERSION 3.10)
project(Cool)

set(CMAKE_CXX_STANDARD 14)

option(BUILD_TESTS "Build the tests" ON)

include_directories(include)

add_subdirectory(src)

#if (BUILD_TESTS)
    include(GoogleTest)
    include_directories(tests)
    enable_testing()
    add_subdirectory(tests)
#endif()
