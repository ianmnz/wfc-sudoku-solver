#include <iostream>
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
        std::cerr << "File " << filename << " not found." << std::endl;
    }

    return grids;
}


int main()
{
    const auto& grids = parse("sudoku10k.txt");

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

    int count = 0;

    for (const auto& grid : grids) {
        sudoku::q_board b(grid);
        if (sudoku::solve(b)) {
            sudoku::print_side_by_side(grid, b.show());

        } else {
            std::cout << "No solution found for " << grid << std::endl;
        }
        if (++count >= 1) break;
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << " s" << std::endl;

    return 0;
}
