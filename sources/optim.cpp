#include "optim.hpp"

#ifdef ORTOOLS

#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "ortools/linear_solver/linear_solver.h"
#include "ortools/sat/cp_model.h"

#include "utils.hpp"


/**
 * @brief A Constraint Programming Sudoku Solver
 *
 * @param board Sudoku board encoded in string format
 * @return true if solved,
 * @return false if not
 */
bool sudoku::cp::solve(std::string& board)
{
    using namespace operations_research;
    using namespace sat;

    // Constraint Programming Model
    CpModelBuilder cp_model;

    const Domain digits(1, N);
    std::array<IntVar, N * N> tiles;

    for (int idx = 0; idx < N * N; ++idx) {
        // Create variables
        tiles[idx] = cp_model.NewIntVar(digits);

        const char c_digit = board[idx];
        if (c_digit != '.') {
            // Fix variables to already set digits
            const int i_digit = c_digit - '0';
            cp_model.FixVariable(tiles[idx], i_digit);
        }
    }

    // For each row
    std::vector<IntVar> row;
    row.reserve(N);
    for (int i = 0; i < N; ++i)
    {
        row.clear();
        for (int j = 0; j < N; ++j) {
            row.push_back(tiles[utils::grid2array(i, j)]);
        }
        // Constraint all values different
        cp_model.AddAllDifferent(row);
    }

    // For each column
    std::vector<IntVar> col;
    col.reserve(N);
    for (int j = 0; j < N; ++j)
    {
        col.clear();
        for (int i = 0; i < N; ++i) {
            col.push_back(tiles[utils::grid2array(i, j)]);
        }
        // Constraint all values different
        cp_model.AddAllDifferent(col);
    }

    // For each box
    std::vector<IntVar> box;
    box.reserve(N);
    for (int r = 0; r < N; r += BOX) {
        for (int c = 0; c < N; c += BOX)
        {
            box.clear();
            for (int i = 0; i < BOX; ++i) {
                for (int j = 0; j < BOX; ++j) {
                    box.push_back(tiles[utils::grid2array(r + i, c + j)]);
                }
            }
            // Constraint all values different
            cp_model.AddAllDifferent(box);
        }
    }

    // Solve
    const CpSolverResponse response = Solve(cp_model.Build());

    if (response.status() != CpSolverStatus::OPTIMAL &&
        response.status() != CpSolverStatus::FEASIBLE) {
        return false;
    }

    // Output
    std::ostringstream oss;
    for (const auto& tile : tiles) {
        oss << SolutionIntegerValue(response, tile);
    }
    board = oss.str();

    return true;
}


/**
 * @brief A Mixed Integer Linear Programming Sudoku Solver
 *
 * @param board Sudoku board encoded in string format
 * @return true if solved,
 * @return false if not
 */
bool sudoku::lp::solve(std::string& board)
{
    using namespace operations_research;

    // Mixed Integer Linear Programming Model
    std::unique_ptr<MPSolver> mp_solver(MPSolver::CreateSolver("SCIP"));

    if (!mp_solver) {
        std::cerr << "SCIP solver unavailable.\n";
        return false;
    }

    std::array<std::vector<MPVariable*>, N * N> tiles;

    for (int idx = 0; idx < N * N; ++idx) {
        // Create variables
        mp_solver->MakeBoolVarArray(N, "", &tiles[idx]);

        const char c_digit = board[idx];
        if (c_digit != '.') {
            const int i_digit = c_digit - '0';

            // Constraint set value
            LinearExpr tile_set{tiles[idx][i_digit - 1]};
            mp_solver->MakeRowConstraint(tile_set == 1);
        }
    }

    // For each tile
    for (const auto tile : tiles)
    {
        LinearExpr sum_digits;
        for (const auto d : tile) {
            sum_digits += d;
        }
        // Constraint set exactly one value
        mp_solver->MakeRowConstraint(sum_digits == 1);
    }

    // For each digit
    for (int d = 0; d < N; ++d)
    {
        // For each row
        for (int i = 0; i < N; ++i)
        {
            LinearExpr sum_col;
            for (int j = 0; j < N; ++j) {
                sum_col += tiles[utils::grid2array(i, j)][d];
            }
            // Constraint all values different
            mp_solver->MakeRowConstraint(sum_col == 1);
        }

        // For each column
        for (int j = 0; j < N; ++j)
        {
            LinearExpr sum_row;
            for (int i = 0; i < N; ++i) {
                sum_row += tiles[utils::grid2array(i, j)][d];
            }
            // Constraint all values different
            mp_solver->MakeRowConstraint(sum_row == 1);
        }

        // For each box
        for (int r = 0; r < N; r += BOX) {
            for (int c = 0; c < N; c += BOX)
            {
                LinearExpr sum_box;
                for (int i = 0; i < BOX; ++i) {
                    for (int j = 0; j < BOX; ++j) {
                        sum_box += tiles[utils::grid2array(i, j)][d];
                    }
                }
                // Constraint all values different
                mp_solver->MakeRowConstraint(sum_box == 1);
            }
        }
    }

    // Solve
    const MPSolver::ResultStatus result = mp_solver->Solve();

    if (result != MPSolver::OPTIMAL &&
        result != MPSolver::FEASIBLE) {
        return false;
    }

    // Output
    std::ostringstream oss;
    for (const auto& tile : tiles) {
        for (int d = 0; d < N; ++d) {
            if (tile[d]->solution_value() > 0) {
                oss << d + 1;
                break;
            }
        }
    }
    board = oss.str();

    return true;
}

#endif
