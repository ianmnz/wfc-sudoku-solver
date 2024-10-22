# Wave Function Collapse Sudoku solver CMake file

cmake_minimum_required(VERSION 3.12)

project("WFC Sudoku solver"
        LANGUAGES CXX
)

add_executable(sudoku sources/main.cpp)

target_compile_features(sudoku PRIVATE cxx_std_17)
target_compile_options(sudoku PRIVATE -Og)

target_include_directories(sudoku PRIVATE sources)
target_sources(sudoku
               PRIVATE sources/bit_manipulation.cpp
               PRIVATE sources/board.cpp
               PRIVATE sources/sudoku.cpp
)
