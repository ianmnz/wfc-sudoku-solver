#include "board.hpp"

#include <sstream>


/**
 * @brief Get same column tiles
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 column peers
 */
std::array<int, N - 1> get_col_peers(const int i, const int j)
{
    std::array<int, N - 1> columns;

    int idx = 0;
    for (int r = 0; r < N; ++r) {
        if (r == i) {
            continue;
        }
        columns[idx++] = grid2array(r, j);
    }
    return columns;
}

/**
 * @brief Get same row tiles
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 row peers
 */
std::array<int, N - 1> get_row_peers(const int i, const int j)
{
    std::array<int, N - 1> rows;

    int idx = 0;
    for (int c = 0; c < N; ++c) {
        if (c == j) {
            continue;
        }
        rows[idx++] = grid2array(i, c);
    }
    return rows;
}

/**
 * @brief Get same box tiles
 *
 * @param i row id
 * @param j column id
 * @return Array of N-1 box peers
 */
std::array<int, N - 1> get_box_peers(const int i, const int j)
{
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
            boxes[idx++] = grid2array(r, c);
        }
    }
    return boxes;
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
    if (has_zero_entropy())
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
        if (bit::check(_superposition, p - 1)) {
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
    static constexpr std::string_view digits{"123456789"};

    for (int idx = 0; idx < N * N; ++idx) {
        const char c_digit = grid[idx];

        if (digits.find(c_digit) != std::string::npos) {
            const int i_digit = c_digit - '0';

            collapse(idx, i_digit);
        }
    }
}

/**
 * @brief Output board
 *
 * @return The string format of board
 */
std::string q_board::show() const
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
    const auto& [i, j] = array2grid(index);
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

    if (tile.has_zero_entropy()) {
        // Found inconsistency
        // Abort collapsing
        return false;
    }

    // Collapse cascade
    if (tile.get_entropy() == 1) {
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
    // Checks if digit is the only possible candidate for a given
    // tile in a peer group
    for (int d = 1; d <= N; ++d) {
        int inferred_idx = -1;
        bool is_inferred_idx_uniq = false;

        for (const int idx : arr) {
            const q_tile& tile = _grid[idx];

            if (tile.get_superposition()[d - 1]) {
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
