#include "board.hpp"

#include <sstream>

namespace sudoku
{

int q_tile::get_entropy() const
{
    if (!_superposition)
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

void q_board::collapse(const int index, const int value)
{
    const auto& [i, j] = array2grid(index);

    propagate_col(j, value);
    propagate_row(i, value);
    propagate_box(i, j, value);

    auto& tile = _grid[index];
    tile.fill(value);
}

void q_board::propagate_col(const int j, const int value)
{
    for (int i = 0; i < N; ++i) {
        _grid[grid2array(i, j)].eliminate(value);
    }
}

void q_board::propagate_row(const int i, const int value)
{
    for (int j = 0; j < N; ++j) {
        _grid[grid2array(i, j)].eliminate(value);
    }
}

void q_board::propagate_box(const int i, const int j, const int value)
{
    const int ii = i - (i % BOX);
    const int jj = j - (j % BOX);

    for (int di = 0; di < BOX; ++di) {
        for (int dj = 0; dj < BOX; ++dj) {
            _grid[grid2array(ii + di, jj + dj)].eliminate(value);
        }
    }
}

} // namespace sudoku
