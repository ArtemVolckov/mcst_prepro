cmake_minimum_required(VERSION 3.15)
project(prepro CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS program_options)

add_library(prepro_lib STATIC
    prepro/lexer.cpp
    prepro/parser.cpp
    prepro/preprocessor.cpp
)

target_compile_features(prepro_lib PUBLIC cxx_std_20)

target_include_directories(prepro_lib PUBLIC .)

add_executable(pp prepro/main.cpp)
target_link_libraries(pp PRIVATE prepro_lib Boost::program_options)

add_executable(lexdump tests/lexdump.cpp)
target_link_libraries(lexdump prepro_lib)

add_executable(parsedump tests/parsedump.cpp)
target_link_libraries(parsedump prepro_lib)

add_executable(lexer_tests tests/lexer_tests.cpp)
target_link_libraries(lexer_tests prepro_lib)