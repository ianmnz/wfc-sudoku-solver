#include "board.hpp"

#include <unordered_map>
#include <sstream>


/**
 * @brief Get same column tiles (and cache the result)
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 column peers
 */
std::array<int, N - 1> get_col_peers(const int i, const int j)
{
    static std::unordered_map<int, std::array<int, N - 1>> cache;

    const int key = utils::grid2array(i, j);
    if (auto found = cache.find(key); found != cache.end()) {
        return found->second;
    }

    std::array<int, N - 1> columns;

    int idx = 0;
    for (int r = 0; r < N; ++r) {
        if (r == i) {
            continue;
        }
        columns[idx++] = utils::grid2array(r, j);
    }
    return cache[key] = columns;
}

/**
 * @brief Get same row tiles (and cache the result)
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 row peers
 */
std::array<int, N - 1> get_row_peers(const int i, const int j)
{
    static std::unordered_map<int, std::array<int, N - 1>> cache;

    const int key = utils::grid2array(i, j);
    if (auto found = cache.find(key); found != cache.end()) {
        return found->second;
    }

    std::array<int, N - 1> rows;

    int idx = 0;
    for (int c = 0; c < N; ++c) {
        if (c == j) {
            continue;
        }
        rows[idx++] = utils::grid2array(i, c);
    }
    return cache[key] = rows;
}

/**
 * @brief Get same box tiles (and cache the result)
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 box peers
 */
std::array<int, N - 1> get_box_peers(const int i, const int j)
{
    static std::unordered_map<int, std::array<int, N - 1>> cache;

    const int key = utils::grid2array(i, j);
    if (auto found = cache.find(key); found != cache.end()) {
        return found->second;
    }

    std::array<int, N - 1> boxes;

    const int rr = i - (i % BOX);  // = (int)(i / BOX) * BOX
    const int cc = j - (j % BOX);  // = (int)(j / BOX) * BOX

    int idx = 0;
    for (int dr = 0; dr < BOX; ++dr) {
        const int r = rr + dr;

        for (int dc = 0; dc < BOX; ++dc) {
            const int c = cc + dc;

            if (r == i && c == j) {
                continue;
            }
            boxes[idx++] = utils::grid2array(r, c);
        }
    }
    return cache[key] = boxes;
}


namespace sudoku
{

/**
 * @brief Get entropy value
 *
 * @return Entropy
 */
int q_tile::get_entropy() const
{
    if (!(_superposition & init_state))
        return 0;

    return bit::count(_superposition);
}

/**
 * @brief All possible candidates to the tile
 *
 * @return Array of candidates
 */
const std::vector<int> q_tile::get_possibilities() const
{
    std::vector<int> possibilities;
    possibilities.reserve(N);

    for (int p = 1; p <= N; ++p) {
        if (is_possible(p)) {
            possibilities.push_back(p);
        }
    }
    return possibilities;
}


/**
 * @brief Construct a new q board::q board object
 *
 * @param grid A N*N string of numbers and blank spaces
 */
q_board::q_board(std::string_view grid)
{
    for (int idx = 0; idx < N * N; ++idx) {
        const char c_digit = grid[idx];

        if (c_digit != '.') {
            const int i_digit = c_digit - '0';

            collapse(idx, i_digit);
        }
    }
}

/**
 * @brief Pre-computes col, row and box peers of a tile
 *
 * @param i row id
 * @param j col id
 */
void q_board::precompute_peers(const int i, const int j)
{
    get_col_peers(i, j);
    get_row_peers(i, j);
    get_box_peers(i, j);
}

/**
 * @brief Output board
 *
 * @return The string format of board
 */
std::string q_board::serialize() const
{
    std::stringstream ss;
    for (const auto tile : _grid) {
        if (tile.has_collapsed()) {
            ss << tile.get_digit();

        } else {
            ss << '.';
        }
    }
    return ss.str();
}

/**
 * @brief Sets a tile to a specific value, propagates this informations among
 * its peers and infers next possible collapse
 *
 * @param index The tile index
 * @param digit The chosen value
 * @return true if tile was set to value,
 * @return false otherwise
 */
bool q_board::collapse(const int index, const int digit)
{
    const auto& [i, j] = utils::array2grid(index);
    q_tile& tile = _grid[index];

    if (tile.has_collapsed()) {
        // Asserts that collapsed tile has the correct value
        return (tile.get_digit() == digit);
    }

    // Sets tile value
    tile.fill(digit);

    // Propagates collapse information
    if (!propagate_col(i, j, digit)
        || !propagate_row(i, j, digit)
        || !propagate_box(i, j, digit))
    {
        return false;
    }

    // Infers subsequent collapses
    if (!infer_col(i, j)
        || !infer_row(i, j)
        || !infer_box(i, j))
    {
        return false;
    }

    return true;
}

/**
 * @brief Recursively propagates collapse information by removing value as
 * possibility
 *
 * @param idx Index of peer tile tha receives information
 * @param digit Value to which original tile was set
 * @return true if information was properly propagated,
 * @return false otherwise
 */
bool q_board::propagate(const int idx, const int digit)
{
    q_tile& tile = _grid[idx];

    // Removes value as possibility
    tile.eliminate(digit);

    const int entropy = tile.get_entropy();

    if (!entropy) {
        // Found inconsistency
        // Abort collapsing
        return false;
    }

    // Collapse cascade
    // Tile not set but only has one option
    if (entropy == 1) {
        return collapse(idx, tile.get_digit());
    }
    return true;
}

/**
 * @brief Recursively infers if original collapse forced another collapse to a
 * peer group
 *
 * @param arr Peer group of original tile
 * @return false if inference results in inconsistency,
 * @return true otherwise
 */
bool q_board::infer(const std::array<int, N - 1>& arr)
{
    // Checks if there is only one tile that can
    // hold a digit, and sets the digit to the tile
    for (int d = 1; d <= N; ++d) {
        int inferred_idx = -1;
        bool is_inferred_idx_uniq = false;

        for (const int idx : arr) {
            const q_tile& tile = _grid[idx];

            if (tile.is_possible(d)) {
                if (is_inferred_idx_uniq) {
                    is_inferred_idx_uniq = false;
                    break;

                } else {
                    is_inferred_idx_uniq = true;
                    inferred_idx = idx;
                }
            }
        }

        if (!is_inferred_idx_uniq) {
            continue;
        }

        if (_grid[inferred_idx].has_collapsed()) {
            continue;
        }

        // Checks if inference creates inconsistency
        if (!collapse(inferred_idx, d)) {
            return false;
        }
    }

    return true;
}

bool q_board::propagate_col(const int i, const int j, const int digit)
{
    for (const int idx : get_col_peers(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::propagate_row(const int i, const int j, const int digit)
{
    for (const int idx : get_row_peers(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::propagate_box(const int i, const int j, const int digit)
{
    for (const int idx : get_box_peers(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::infer_col(const int i, const int j)
{
    return infer(get_col_peers(i, j));
}

bool q_board::infer_row(const int i, const int j)
{
    return infer(get_row_peers(i, j));
}

bool q_board::infer_box(const int i, const int j)
{
    return infer(get_box_peers(i, j));
}

}  // namespace sudoku
