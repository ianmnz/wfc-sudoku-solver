#include <chrono>
#include <ctype.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "sudoku.hpp"
#include "utils.hpp"


/**
 * @brief Checks if string is a number
 *
 * @param sv Possible number written as a string
 * @return true is a number,
 * @return false otherwise
 */
bool is_numeric(const std::string_view sv)
{
    return (!sv.empty() &&
            std::find_if(sv.begin(), sv.end(), [](unsigned char c) {
                return !std::isdigit(c);
            }) == sv.end());
}

/**
 * @brief Parse command line
 *
 * @param argc Nb of arguments
 * @param argv Arguments
 * @param path Reference to file of puzzles
 * @param nb_threads Reference to chosen nb of threads
 * @param print_boards Reference to flag of showing boards
 */
void parse_cmdline(const int argc, const char** argv,
                   std::filesystem::path& path, int& nb_threads,
                   bool& print_boards)
{
    // Default values
    path = "data/sudoku10k.txt";
    nb_threads = 4;
    print_boards = true;

    switch (argc) {
    case 4:
        if (std::string(argv[3]) == "0") {
            print_boards = false;
        }
    case 3:
        if (is_numeric(argv[2])) {
            nb_threads = std::min(std::stoi(argv[2]),
                                  (int)std::thread::hardware_concurrency());
        }
    case 2:
        path = argv[1];
    default:
        break;
    }
}

/**
 * @brief Parse file of grids
 *
 * @param path File
 * @return Array of grids
 */
std::vector<std::string> parse_grids(const std::filesystem::path& path)
{
    std::vector<std::string> grids;
    std::ifstream infile(path);

    if (infile.is_open()) {
        std::string grid;

        while (std::getline(infile, grid)) {
            grids.push_back(grid);
        }

    } else {
        std::cerr << "File '" << path << "' not found." << std::endl;
    }

    return grids;
}


int main(int argc, const char* argv[])
{
    std::filesystem::path path;
    int nb_threads;
    bool print_boards;
    std::mutex cout_mtx;

    parse_cmdline(argc, argv, path, nb_threads, print_boards);
    const auto& grids = parse_grids(path);
    nb_threads = std::min(nb_threads, (int)grids.size());
    print_boards &= (grids.size() <= 10); // Arbitrary magic number to avoid printing too many boards

    std::cout << "Sudoku solver: "
              << grids.size() << " puzzles to solve on "
              << nb_threads << " threads" << std::endl;

    // Start chrono
    std::chrono::high_resolution_clock::time_point begin =
        std::chrono::high_resolution_clock::now();

    {
        utils::thread_pool pool(nb_threads);

        for (const auto& grid : grids) {
            pool.enqueue([grid, print_boards, &cout_mtx] {
                sudoku::q_board game(grid);
                if (sudoku::solve(game)) {
                    if (print_boards) {
                        sudoku::print_side_by_side(grid, game.show(), cout_mtx);
                    }
                } else {
                    std::lock_guard<std::mutex> lock(cout_mtx);
                    std::cout << "No solution found for " << grid << std::endl;
                }
            });
        }
    }

    // End chrono
    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();

    std::cout << "Elapsed time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << " s" << std::endl;

    return 0;
}
