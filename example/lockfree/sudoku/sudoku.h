#ifndef SUDOKU_SUDOKU_H
#define SUDOKU_SUDOKU_H

#include "Types.h"

// FIXME, use (const char*, len) for saving memory copying.
kimgbo::string solveSudoku(const kimgbo::string& puzzle);
const int kCells = 81;

#endif
