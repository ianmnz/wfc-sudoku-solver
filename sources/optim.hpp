#pragma once

#ifdef ORTOOLS

#include <string>

namespace sudoku
{

namespace cp
{
bool solve(std::string& board);
}  // namespace cp

namespace lp
{
bool solve(std::string& board);
}  // namespace lp

}  // namespace sudoku

#endif