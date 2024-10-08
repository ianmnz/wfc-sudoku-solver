#include <iostream>
#include <ctype.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

#include "sudoku.hpp"


std::vector<std::string> parse(const std::string& filename)
{
    std::vector<std::string> grids;
    std::ifstream infile(filename);

    if (infile.is_open()) {
        std::string grid;

        while (std::getline(infile, grid)) {
            grids.push_back(grid);
        }
    } else {
        std::cerr << "File '" << filename << "' not found." << std::endl;
    }

    return grids;
}


int main(int argc, char* argv[])
{
    std::string filename {"sudoku10k.txt"};
    int count = 0, max_count = 3;
    bool should_print_boards {true};

    if (argc > 1) filename = argv[1];
    if (argc > 2 && isdigit(argv[2][0])) max_count = std::stoi(argv[2]);
    if (argc > 3) should_print_boards = false;

    const auto& grids = parse(filename);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

    for (const auto& grid : grids) {
        sudoku::q_board b(grid);
        if (sudoku::solve(b)) {
            if (should_print_boards) sudoku::print_side_by_side(grid, b.show());

        } else {
            std::cout << "No solution found for " << grid << std::endl;
        }
        if (++count >= max_count) break;
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << " s" << std::endl;

    return 0;
}
