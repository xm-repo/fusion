
cmake_minimum_required(VERSION 3.10)

option(BUILD_TESTS "Build test program" ON)

SET(TEST_NAME "TestMegaBI")
SET(TARGET_NAME "MegaBI")
SET(PROJECT_NAME "MegaBI")

project(${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/MegaBI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ThreadPool.hpp"
    )

set(TEST_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp"
    )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_executable(${TARGET_NAME} ${SOURCE_FILES})

if(BUILD_TESTS)
    add_executable(${TEST_NAME} ${TEST_SOURCE_FILES})    
    enable_testing()
    add_test(NAME TestMegaBI COMMAND TestMegaBI)
endif(BUILD_TESTS)

find_package (Threads REQUIRED)
target_link_libraries(${TARGET_NAME} Threads::Threads)

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -O3 -Wall -Wextra -pedantic -Werror)
endif()
