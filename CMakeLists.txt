cmake_minimum_required (VERSION 3.10)

include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 14)

add_executable(sushiscript ${CMAKE_SOURCE_DIR}/src/main.cpp)