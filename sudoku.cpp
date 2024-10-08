#include "sudoku.hpp"

#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <algorithm>

#include "random.hpp"


namespace sudoku
{

void print_side_by_side(std::string_view init_grid, std::string_view solved_grid)
{
    static constexpr std::string_view row_separator = " -----+-----+-----";

    std::cout << "\n";

    for (int i = 0; i < N; ++i) {

        if (i == 3 || i == 6) {
            std::cout << row_separator << "\t" << row_separator << "\n";
        }

        for (int j = 0; j < N; ++j) {
            std::cout << (j == 3 || j == 6 ? '|' : ' ') << init_grid[grid2array(i, j)];
        }

        std::cout << "\t";

        for (int j = 0; j < N; ++j) {
            std::cout << (j == 3 || j == 6 ? '|' : ' ') << solved_grid[grid2array(i, j)];
        }

        std::cout << "\n";
    }

    std::cout << std::endl;
}

std::optional<std::vector<int>> get_candidates(const q_board& board)
{
    int min_entropy = 2 * N;

    std::vector<int> candidates;
    candidates.reserve(N * N);

    for (int index = 0; index < (N * N); index++) {
        const q_tile& tile = board.get_tile(index);

        if (tile.has_collapsed()) {
            continue;
        }

        const int entropy = tile.get_entropy();

        if (!entropy) {
            return std::nullopt;
        }

        if (entropy > min_entropy) {
            continue;
        }

        if (entropy < min_entropy) {
            min_entropy = entropy;
            candidates.clear();
        }
        candidates.push_back(index);
    }

    return candidates;
}

bool is_solution(const q_board& board)
{
    return std::all_of(board.get_grid().cbegin(), board.get_grid().cend(),
        [](const q_tile& t){ return t.has_collapsed(); });
}

bool solve(q_board &board)
{
    std::stack<q_board> stk;
    stk.push(board);

    while (!stk.empty()) {
        q_board curr = stk.top();
        stk.pop();

        const auto opt_candidates = get_candidates(curr);

        if (!opt_candidates.has_value()) {
            continue;
        }

        const std::vector<int>& candidates = opt_candidates.value();

        if (candidates.empty()) {
            board = curr;
            return true;
        }

        const int chosen_idx = randomns::sample(candidates)[0];
        std::vector<int> possibilities = curr.get_tile(chosen_idx).get_possibilities();

        randomns::shuffle(possibilities);   // Won't be needed when parallelized
        for (int chosen_val : possibilities) {
            stk.emplace(curr);  // Creates a copy directly at the top of the stack

            if (!stk.top().collapse(chosen_idx, chosen_val)) {
                stk.pop();

            } else if (is_solution(stk.top())) {
                board = stk.top();
                return true;
            }
        }
    }

    return false;
}

} // namespace sudoku
