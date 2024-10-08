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
            ss << tile.get_value();

        } else {
            ss << '.';
        }
    }
    return ss.str();
}

bool q_board::collapse(const int index, const int value)
{
    const auto& [i, j] = array2grid(index);

    _grid[index].fill(value);

    if (!propagate_col(i, j, value)
        || !propagate_row(i, j, value)
        || !propagate_box(i, j, value))
    {
        return false;
    }

    return true;
}

bool q_board::propagate(const int idx, const int value)
{
    _grid[idx].eliminate(value);

    if (_grid[idx].has_zero_entropy()) {
        return false;
    }
    return true;
}

bool q_board::propagate_col(const int i, const int j, const int value)
{
    for (int r = 0; r < N; ++r) {
        if (r == i) {
            continue;
        }
        if (!propagate(grid2array(r, j), value)) {
            return false;
        }
    }

    return true;
}

bool q_board::propagate_row(const int i, const int j, const int value)
{
    for (int c = 0; c < N; ++c) {
        if (c == j) {
            continue;
        }
        if (!propagate(grid2array(i, c), value)) {
            return false;
        }
    }

    return true;
}

bool q_board::propagate_box(const int i, const int j, const int value)
{
    const int rr = i - (i % BOX);   // = (int)(i / BOX) * BOX
    const int cc = j - (j % BOX);   // = (int)(j / BOX) * BOX

    for (int dr = 0; dr < BOX; ++dr) {
        const int r = rr + dr;

        for (int dc = 0; dc < BOX; ++dc) {
            const int c = cc + dc;

            if (r == i && c == j) {
                continue;
            }
            if (!propagate(grid2array(r, c), value)) {
                return false;
            }
        }
    }

    return true;
}

} // namespace sudoku
