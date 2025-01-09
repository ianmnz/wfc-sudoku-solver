#include "sudoku.hpp"

#include <optional>
#include <stack>

#include "utils.hpp"


namespace sudoku
{

/**
 * @brief Initialises sudoku solver by pre-computing peers
 *
 */
void wfc::init()
{
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            q_board::precompute_peers(i, j);
        }
    }
}

/**
 * @brief Get the tiles with minimal entropy
 *
 * @param board Current board reference
 * @return std::optional vector of tiles
 */
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
            // No solution possible
            // due to a conflict of collapsed tiles
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

    // Returns an empty vector
    // if all tiles have collapsed
    // i.e., if is a solution
    candidates.shrink_to_fit();
    return candidates;
}

/**
 * @brief A DFS Sudoku Solver (with backtracking)
 *
 * @param board Sudoku board reference that will be filled with the solution
 * @return true if solved,
 * @return false if not
 */
bool wfc::solve(q_board& board)
{
    std::stack<q_board> stk;
    stk.push(board); // Pushes a copy of board to the top of the stack

    while (!stk.empty()) {
        q_board curr = stk.top();
        stk.pop();

        const auto opt_candidates = get_candidates(curr);

        if (!opt_candidates.has_value()) {
            // Found a state with no possible solution
            // Backtracks to previous state
            continue;
        }

        const std::vector<int>& candidates = opt_candidates.value();

        if (candidates.empty()) {
            // Found a solution
            // Updates the board and returns
            board = curr;
            return true;
        }

        // Chooses randomly a tile among the candidates to collapse
        const int chosen_idx = utils::sample(candidates);
        std::vector<int> possibilities = curr.get_tile(chosen_idx).get_possibilities();

        // Not actually necessary but prevents always trying the same
        // order of possibilities over and over again
        utils::shuffle(possibilities);

        for (int chosen_val : possibilities) {
            // Since 'curr' is a reference,
            // creates a copy directly on the top of the stack
            // (avoiding unnecessary copies when pushing)
            stk.emplace(curr);

            // Checks if tile can be collapsed to chosen value
            // If not, pop state from stack
            if (!stk.top().collapse(chosen_idx, chosen_val)) {
                stk.pop();
            }
        }
    }

    return false;
}

}  // namespace sudoku
