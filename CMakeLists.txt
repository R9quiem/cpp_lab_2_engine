cmake_minimum_required(VERSION 3.16)
project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(engine_lib INTERFACE)
target_include_directories(engine_lib INTERFACE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
)

add_executable(engine_app src/main.cpp)
target_link_libraries(engine_app PRIVATE engine_lib)

add_executable(engine_tests tests/EngineTests.cpp)
target_link_libraries(engine_tests PRIVATE engine_lib GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(engine_tests)
