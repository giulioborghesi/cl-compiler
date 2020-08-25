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

add_executable(cool ./src/exec/cool.cpp)
target_link_libraries(cool LINK_PUBLIC "lib_analysis;lib_frontend;lib_codegen;lib_core;lib_ir")
