cmake_minimum_required(VERSION 3.15)
project(prepro CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(prepro_lib STATIC
    prepro/lexer.cpp
    prepro/parser.cpp
)

add_executable(lexdump tests/lexdump.cpp)
target_link_libraries(lexdump prepro_lib)

add_executable(parsedump tests/parsedump.cpp)
target_link_libraries(parsedump prepro_lib)

add_executable(lexer_tests tests/lexer_tests.cpp)
target_link_libraries(lexer_tests prepro_lib)