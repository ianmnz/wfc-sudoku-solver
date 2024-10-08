#pragma once

#include <array>
#include <vector>
#include <bitset>

#include "bit_manipulation.hpp"

#define N 9
#define BOX 3
#define INIT_STATE 0b0111111111


inline std::array<int, 2> array2grid(const int index) { return {(index / N), (index % N)}; }
inline int grid2array(const int i, const int j) { return i * N + j; }


namespace sudoku
{

class q_tile final
{
public:
    inline bool has_collapsed() const { return _superposition & (1 << N); }
    inline bool has_zero_entropy() const { return !(_superposition & INIT_STATE); }

    inline std::bitset<N + 1> get_superposition() const { return std::bitset<N + 1>(_superposition); }
    inline int get_value() const { return bit::mask.at(_superposition); }

    int get_entropy() const;
    const std::vector<int> get_possibilities() const;

    inline void fill(const int value) { _superposition = bit::set(1 << N, value - 1); }
    inline void eliminate(const int value) { _superposition = bit::clear(_superposition, value - 1); }
    inline void clear() { _superposition = INIT_STATE; }

private:
    int16_t _superposition = INIT_STATE;
};


class q_board final
{
public:
    q_board() = default;
    q_board(std::string_view grid);

    inline const std::array<q_tile, N * N>& get_grid() const { return _grid; }
    inline const q_tile& get_tile(const int index) const { return _grid[index]; }

    std::string show() const;
    bool collapse(const int idx, const int value);

private:
    bool propagate(const int idx, const int value);
    bool propagate_col(const int i, const int j, const int value);
    bool propagate_row(const int i, const int j, const int value);
    bool propagate_box(const int i, const int j, const int value);

    std::array<q_tile, N * N> _grid;
};

} // namespace sudoku
