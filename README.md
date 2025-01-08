# WFC Sudoku solver

A simple example of a Wave Function Collapse algorithm for solving Sudoku games.
Since each game is independent, uses concurrency for solving multiple boards faster.

### References

Wave function collapse inspired by: https://www.youtube.com/watch?v=2SuvO4Gi7uY

Algorithm and benchmark based on: https://github.com/norvig/pytudes/blob/main/ipynb/Sudoku.ipynb

Thread pool based on: https://www.geeksforgeeks.org/thread-pool-in-cpp/

### P.S.
I was curious about using Operational Research to solve the same problem.
Here is a reference of model it using Google's OR-Tools in Python:
https://www.kaggle.com/code/pintowar/modeling-a-sudoku-solver-with-or-tools
