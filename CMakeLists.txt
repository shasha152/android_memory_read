cmake_minimum_required(VERSION 3.10.0)
project(android_memory_lib VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

set(DUSE_AMEMORY_TEST 1)

file(GLOB SOURCES "src/*.cpp")

add_subdirectory(third-party/pfr)

add_library(android_memory STATIC ${SOURCES})
target_include_directories(android_memory PUBLIC ./include)
target_link_libraries(android_memory PRIVATE c++_static)
target_link_libraries(android_memory PUBLIC Boost::pfr)

IF (USE_AMEMORY_TEST)
    file(GLOB TEST_SOURCES "test/pros/*.cpp")
    add_executable(memory_test test/main.cpp ${TEST_SOURCES})
    target_link_libraries(memory_test PRIVATE android_memory)
endif ()