#pragma once

#include <array>
#include <vector>

#include "bit_manipulation.hpp"
#include "utils.hpp"


namespace sudoku
{

inline constexpr int16_t init_state = 0b01'1111'1111;

class q_tile final
{
public:
    inline bool has_collapsed() const { return bit::check(_superposition, N); }

    inline int get_digit() const { return bit::mask.at(_superposition); }
    int get_entropy() const;

    inline bool is_possible(const int digit) const { return bit::check(_superposition, digit - 1); }
    const std::vector<int> get_possibilities() const;

    inline void fill(const int digit) { _superposition = bit::set(1 << N, digit - 1); }
    inline void eliminate(const int digit) { _superposition = bit::clear(_superposition, digit - 1); }

private:
    int16_t _superposition = init_state;
};


class q_board final
{
public:
    q_board() = default;
    q_board(std::string_view grid);

    inline const std::array<q_tile, N * N>& get_grid() const { return _grid; }
    inline const q_tile& get_tile(const int index) const { return _grid[index]; }

    static void precompute_peers(const int i, const int j);

    std::string serialize() const;
    bool collapse(const int idx, const int digit);

private:
    bool propagate(const int idx, const int digit);
    bool propagate_col(const int i, const int j, const int digit);
    bool propagate_row(const int i, const int j, const int digit);
    bool propagate_box(const int i, const int j, const int digit);

    bool infer(const std::array<int, N - 1>& arr);
    bool infer_col(const int i, const int j);
    bool infer_row(const int i, const int j);
    bool infer_box(const int i, const int j);

    std::array<q_tile, N * N> _grid;
};

}  // namespace sudoku
