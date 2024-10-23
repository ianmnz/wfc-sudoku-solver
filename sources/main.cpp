#include <iostream>
#include <ctype.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

#include "sudoku.hpp"
#include "utils.hpp"


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
    std::string filename {"data/sudoku10k.txt"};
    int count = 0, max_count = 3;
    bool should_print_boards {true};
    std::mutex cout_mtx;

    if (argc > 1) filename = argv[1];
    if (argc > 2 && isdigit(argv[2][0])) max_count = std::stoi(argv[2]);
    if (argc > 3) should_print_boards = false;

    const auto& grids = parse(filename);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    {
        utils::thread_pool pool(4);

        for (const auto& grid : grids) {
            pool.enqueue([grid, should_print_boards, &cout_mtx]{
                sudoku::q_board game(grid);
                if (sudoku::solve(game)) {
                    if (should_print_boards) {
                        sudoku::print_side_by_side(grid, game.show(), cout_mtx);
                    }
                } else {
                    std::lock_guard<std::mutex> lock(cout_mtx);
                    std::cout << "No solution found for " << grid << std::endl;
                }
            });
            if (++count >= max_count) break;
        }
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Solved " << std::min((int)grids.size(), max_count) << " puzzles" << std::endl;
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << " s" << std::endl;

    return 0;
}
