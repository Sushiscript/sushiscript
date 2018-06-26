cmake_minimum_required(VERSION 3.5)

project(sushiscript)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup()

set(SUSHI_TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(SUSHI_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SUSHI_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${SUSHI_HEADER_DIR})
include_directories(${CONAN_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(sushiscript ${CMAKE_SOURCE_DIR}/src/main.cpp)

file(GLOB_RECURSE SUSHI_LEXER_SRC ${SUSHI_SRC_DIR}/sushi/lexer/*.cpp)
file(GLOB LEXER_TEST_SRC ${SUSHI_TEST_DIR}/lexer/*.cpp)

file(GLOB_RECURSE SUSHI_PARSER_SRC ${SUSHI_SRC_DIR}/sushi/parser/*.cpp)
file(GLOB PARSER_TEST_SRC ${SUSHI_TEST_DIR}/parser/*.cpp)

file(GLOB_RECURSE SUSHI_SCOPE_SRC ${SUSHI_SRC_DIR}/sushi/scope/*.cpp)
file(GLOB SCOPE_TEST_SRC  ${SUSHI_TEST_DIR}/scope/*.cpp
    ${SUSHI_TEST_DIR}/scope/visitor/*.cpp)

file(GLOB_RECURSE SUSHI_CODE_GEN_SRC
    ${SUSHI_SRC_DIR}/sushi/code-generation/*.cpp)
file(GLOB CODE_GEN_TEST_SRC ${SUSHI_TEST_DIR}/code-generation/*.cpp)


add_executable(lexer-test ${LEXER_TEST_SRC} ${SUSHI_LEXER_SRC})
add_executable(parser-test
    ${PARSER_TEST_SRC} ${SUSHI_PARSER_SRC} ${SUSHI_LEXER_SRC})
add_executable(scope-test ${SCOPE_TEST_SRC} ${SUSHI_SCOPE_SRC}
    ${SUSHI_PARSER_SRC} ${SUSHI_LEXER_SRC})
add_executable(code-gen-test
    ${CODE_GEN_TEST_SRC} ${SUSHI_PARSER_SRC} ${SUSHI_LEXER_SRC}
    ${SUSHI_CODE_GEN_SRC})

target_link_libraries(lexer-test ${CONAN_LIBS_GTEST})
target_link_libraries(parser-test ${CONAN_LIBS_GTEST})
target_link_libraries(scope-test ${CONAN_LIBS_GTEST})
target_link_libraries(code-gen-test ${CONAN_LIBS_GTEST})
