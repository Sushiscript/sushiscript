cmake_minimum_required (VERSION 3.10)

project(sushiscript)

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CONAN_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(sushiscript ${CMAKE_SOURCE_DIR}/src/main.cpp)

set(SUSHI_TEST_DIR ${CMAKE_SOURCE_DIR}/test)

file(GLOB LEXER_TEST_SRC ${SUSHI_TEST_DIR}/lexer/*.cpp)

add_executable(lexer-test ${LEXER_TEST_SRC})

target_link_libraries(lexer-test ${CONAN_LIBS_GTEST})