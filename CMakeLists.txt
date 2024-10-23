# Wave Function Collapse Sudoku solver CMake file

cmake_minimum_required(VERSION 3.12)

project("WFC Sudoku solver"
        LANGUAGES CXX
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
message(${CMAKE_THREAD_LIBS_INIT}) # pthread library

add_executable(sudoku sources/main.cpp)

target_compile_features(sudoku PRIVATE cxx_std_17)
target_compile_options(sudoku PRIVATE -Og)

target_link_libraries(sudoku PRIVATE Threads::Threads)

target_include_directories(sudoku PRIVATE sources)
target_sources(sudoku
               PRIVATE sources/bit_manipulation.cpp
               PRIVATE sources/board.cpp
               PRIVATE sources/sudoku.cpp
               PRIVATE sources/utils.cpp
)
