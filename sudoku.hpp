#include "board.hpp"


namespace sudoku
{

void print_side_by_side(std::string_view init_grid, std::string_view solved_grid);
bool solve(q_board& board);

} // namespace sudoku
