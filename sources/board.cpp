#include "board.hpp"

#include <sstream>


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

bool q_board::infer(const std::vector<int>& arr)
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
    for (const int idx : get_cols(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::propagate_row(const int i, const int j, const int digit)
{
    for (const int idx : get_rows(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::propagate_box(const int i, const int j, const int digit)
{
    for (const int idx : get_boxes(i, j)) {
        if (!propagate(idx, digit)) {
            return false;
        }
    }
    return true;
}

bool q_board::infer_col(const int i, const int j)
{
    return infer(get_cols(i, j));
}

bool q_board::infer_row(const int i, const int j)
{
    return infer(get_rows(i, j));
}

bool q_board::infer_box(const int i, const int j)
{
    return infer(get_boxes(i, j));
}

std::vector<int> q_board::get_cols(const int i, const int j) const
{
    std::vector<int> columns;
    columns.reserve(N - 1);

    for (int r = 0; r < N; ++r) {
        if (r == i) {
            continue;
        }
        columns.push_back(grid2array(r, j));
    }

    return columns;
}

std::vector<int> q_board::get_rows(const int i, const int j) const
{
    std::vector<int> rows;
    rows.reserve(N - 1);

    for (int c = 0; c < N; ++c) {
        if (c == j) {
            continue;
        }
        rows.push_back(grid2array(i, c));
    }

    return rows;
}

std::vector<int> q_board::get_boxes(const int i, const int j) const
{
    std::vector<int> boxes;
    boxes.reserve(N - 1);

    const int rr = i - (i % BOX);   // = (int)(i / BOX) * BOX
    const int cc = j - (j % BOX);   // = (int)(j / BOX) * BOX

    for (int dr = 0; dr < BOX; ++dr) {
        const int r = rr + dr;

        for (int dc = 0; dc < BOX; ++dc) {
            const int c = cc + dc;

            if (r == i && c == j) {
                continue;
            }

            boxes.push_back(grid2array(r, c));
        }
    }

    return boxes;
}

} // namespace sudoku
