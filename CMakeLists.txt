cmake_minimum_required(VERSION 3.20)
project(feed_handler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a build with symbols AND optimisation. Low-latency code must be
# measured at -O2; timings from a -O0 build are meaningless. -g keeps gdb and
# callgrind able to show source lines.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer")

add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

# Header-only pieces (spsc_queue.hpp, messages.hpp) travel with this target.
add_library(fh_core INTERFACE)
target_include_directories(fh_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fh_core INTERFACE Threads::Threads)

add_executable(publisher src/publisher.cpp)
target_link_libraries(publisher PRIVATE fh_core)

add_executable(receiver src/receiver.cpp)
target_link_libraries(receiver PRIVATE fh_core)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
find_package(GTest)

if(GTest_FOUND)
    enable_testing()
    add_executable(fh_tests tests/test_messages.cpp)
    target_link_libraries(fh_tests PRIVATE fh_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(fh_tests)
else()
    message(STATUS "GTest not found - skipping tests")
endif()
