#include "board.hpp"

#include <sstream>


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

std::array<int, N - 1> get_box_peers(const int i, const int j)
{
    std::array<int, N - 1> boxes;

    const int rr = i - (i % BOX);   // = (int)(i / BOX) * BOX
    const int cc = j - (j % BOX);   // = (int)(j / BOX) * BOX

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

int q_tile::get_entropy() const
{
    if (has_zero_entropy())
        return 0;

    return bit::count(_superposition);
}

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


q_board::q_board(std::string_view grid)
{
    static constexpr std::string_view digits {"123456789"};

    for (int idx = 0; idx < N * N; ++idx) {
        const char c_digit = grid[idx];

        if (digits.find(c_digit) != std::string::npos) {
            const int i_digit = c_digit - '0';

            collapse(idx, i_digit);
        }
    }
}

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

bool q_board::collapse(const int index, const int digit)
{
    const auto& [i, j] = array2grid(index);
    q_tile& tile = _grid[index];

    if (tile.has_collapsed()) {
        return (tile.get_digit() == digit);
    }

    tile.fill(digit);

    if (!propagate_col(i, j, digit)
        || !propagate_row(i, j, digit)
        || !propagate_box(i, j, digit))
    {
        return false;
    }

    if (!infer_col(i, j)
        || !infer_row(i, j)
        || !infer_box(i, j))
    {
        return false;
    }

    return true;
}

bool q_board::propagate(const int idx, const int digit)
{
    q_tile& tile = _grid[idx];
    tile.eliminate(digit);

    // Check if should abort collapsing
    if (tile.has_zero_entropy()) {
        return false;
    }

    // Collapse cascade
    if (tile.get_entropy() == 1) {
        return collapse(idx, tile.get_digit());
    }
    return true;
}

bool q_board::infer(const std::array<int, N - 1>& arr)
{
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

} // namespace sudoku
