cmake_minimum_required(VERSION 3.14)

project(midnight)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/external/googletest)

file(GLOB_RECURSE
    SOURCES
    src/*.h
    src/*.cpp
)
add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${GTEST_ROOT}/googletest
)

add_subdirectory(tests)
