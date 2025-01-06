#include <chrono>
#include <ctype.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "sudoku.hpp"
#include "utils.hpp"


struct arguments {
    std::filesystem::path path{
        "data/sudoku10k.txt"};      // Path to file with sudoku puzzles
    int nb_threads{4};              // Chosen number of threads
    bool should_show_boards{true};  // Display boards flag
    int max_nb_boards_display{10};  // Display threshold to avoid printing too many boards
};


/**
 * @brief Parse command line
 *
 * @param argc Nb of arguments
 * @param argv Array of arguments
 * @param args Reference to arguments object
 * @return Array of grids
 */
std::vector<std::string> parse(const int argc, const char** argv, arguments& args)
{
    // Checks if string is a number
    auto is_numeric = [](std::string_view sv) {
        return (!sv.empty() &&
                std::find_if(sv.begin(), sv.end(), [](unsigned char c) {
                    return !std::isdigit(c);
                }) == sv.end());
    };

    switch (argc) {
    case 4:
        if (std::string(argv[3]) == "0") {
            args.should_show_boards = false;
        }
        [[fallthrough]];

    case 3:
        if (is_numeric(argv[2])) {
            args.nb_threads = std::stoi(argv[2]);
        }
        [[fallthrough]];

    case 2:
        args.path = argv[1];
        [[fallthrough]];

    default:
        break;
    }

    std::vector<std::string> grids;
    std::ifstream file(args.path);

    if (file.is_open()) {
        std::string grid;

        while (std::getline(file, grid)) {
            grids.push_back(grid);
        }

    } else {
        std::cerr << "File '" << args.path << "' not found." << std::endl;
        exit(1);
    }

    // Guards against too many cores to use or too many boards to print
    args.nb_threads = std::min({args.nb_threads, (int)std::thread::hardware_concurrency(), (int)grids.size()});
    args.should_show_boards &= (grids.size() <= args.max_nb_boards_display);

    return grids;
}

/**
 * @brief Solve sudoku boards
 *
 * @param grids Array of grids to solve
 * @param nb_threads Nb of threads to use
 * @param should_show_boards Flag for printing solutions
 * @return Nb of unsolved boards
 */
int run(const std::vector<std::string>& grids, const int nb_threads, const bool should_show_boards)
{
    std::mutex mtx;
    int unsolved = 0;
    utils::thread_pool pool(nb_threads);

    for (const auto& grid : grids) {
        pool.enqueue([grid, should_show_boards, &mtx, &unsolved] {
            sudoku::q_board game(grid);
            if (sudoku::solve(game)) {
                if (should_show_boards) {
                    sudoku::print_side_by_side(grid, game.show(), mtx);
                }
            } else {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "No solution found for " << grid << std::endl;
                unsolved++;
            }
        });
    }

    return unsolved;
}

int main(int argc, const char* argv[])
{
    arguments args;

    const auto& grids = parse(argc, argv, args);

    std::cout << grids.size() << " sudoku puzzles to solve on "
              << args.nb_threads << " threads\n";

    std::chrono::high_resolution_clock::time_point begin =
        std::chrono::high_resolution_clock::now(); // Start chrono

    const int unsolved = run(grids, args.nb_threads, args.should_show_boards);

    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now(); // End chrono

    if (unsolved) std::cout << "Puzzles unsolved: " << unsolved;
    else std::cout << "Solved all puzzles";

    std::cout << "\nRun took "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << "s\n";

    return 0;
}
