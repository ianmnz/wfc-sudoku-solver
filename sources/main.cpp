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
    bool output_solutions{true};    // Write solutions to file flag
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
            args.output_solutions = false;
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

    // Guards against too many/few cores to use or too many boards to print
    args.nb_threads = std::clamp(
        args.nb_threads,
        1,
        std::min((int)std::thread::hardware_concurrency(), (int)grids.size()));

    return grids;
}

/**
 * @brief Solve sudoku boards concurrently on a thread pool
 *
 * @param grids Array of grids to solve
 * @param nb_threads Nb of threads to use
 * @return Array of solved boards
 */
std::vector<std::string> run(const std::vector<std::string>& grids,
                             const int nb_threads)
{
    sudoku::init();

    std::mutex mtx;
    int unsolved = 0;
    std::vector<std::string> solutions(grids.size(), "");

    utils::thread_pool pool(nb_threads);

    for (int i = 0; i < grids.size(); ++i) {
        const auto& grid = grids[i];

        pool.enqueue([i, grid, &mtx, &unsolved, &solutions] {
            sudoku::q_board board(grid);

            if (sudoku::solve(board)) {
                solutions[i] = board.serialize();

            } else {
                std::lock_guard<std::mutex> lock(mtx);
                unsolved++;
            }
        });
    }

    if (unsolved)
        std::cout << "Puzzles not solved: " << unsolved;
    else
        std::cout << "Solved all puzzles";

    return solutions;
}

/**
 * @brief Outputs solutions to file
 *
 * @param grids Original array of grids
 * @param solutions Array of found solutions for each grid
 */
void output(const std::vector<std::string>& grids,
            const std::vector<std::string>& solutions)
{
    std::ofstream file;
    file.open("solutions.txt");

    constexpr std::string_view row_sep = " -----+-----+-----";

    for (int i = 0; i < grids.size(); ++i) {
        const std::string& grid = grids[i];
        const std::string& solution = solutions[i];

        if (solution.empty()) {
            file << "No solution found for Sudoku board " << i << ": " << grid << "\n";
            continue;
        }

        for (int row = 0; row < N; ++row) {

            if (row == 3 || row == 6) {
                file << row_sep << "\t" << row_sep << "\n";
            }

            for (int col = 0; col < N; ++col) {
                file << (col == 3 || col == 6 ? '|' : ' ') << grid[grid2array(row, col)];
            }

            file << "\t";

            for (int col = 0; col < N; ++col) {
                file << (col == 3 || col == 6 ? '|' : ' ') << solution[grid2array(row, col)];
            }

            file << "\n";
        }

        file << "\n";
    }

    file.close();
}


int main(int argc, const char* argv[])
{
    arguments args;

    const auto& grids = parse(argc, argv, args);

    std::cout << grids.size() << " sudoku puzzles to solve on "
              << args.nb_threads << " threads\n";

    std::chrono::high_resolution_clock::time_point begin =
        std::chrono::high_resolution_clock::now();  // Start chrono

    const auto& solutions = run(grids, args.nb_threads);

    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();  // End chrono

    std::cout << "\nRun took "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1.e-6 << "s\n";

    if (args.output_solutions) {
        output(grids, solutions);
    }

    return 0;
}
