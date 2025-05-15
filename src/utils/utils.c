#include "utils.h"
#include "../globals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t ceilDiv(uint32_t numerator, uint32_t denominator) {
  if (denominator == 0) {
    fprintf(stderr, "Devided by zero, stupid... -> %d/%d", numerator, denominator);
    return -1;
  }
  return ((uint64_t)numerator + denominator - 1) / denominator;
}

void closestDivisors(uint32_t size, uint32_t* rows_out, uint32_t* cols_out) {
  uint32_t rows, cols;
  uint32_t best_r = 1, best_c = size;
  uint32_t min_diff = size;

  for (rows = 1; rows * rows <= size; ++rows) {
    if (size % rows == 0) {
      cols = size / rows;
      uint32_t diff = (rows > cols) ? rows - cols : cols - rows;
      if (diff < min_diff) {
        min_diff = diff;
        best_r = rows;
        best_c = cols;
      }
    }
  }

  *rows_out = best_r;
  *cols_out = best_c;
}

uint32_t polynomialModuloEval(uint8_t order, const uint8_t coefficients[], uint8_t x) {
  uint32_t val = 0;

  uint32_t x_pow = 1;
  for (int i = 0; i < order + 1; ++i) {
    val = (val + coefficients[i] * x_pow) % MOD;
    x_pow = (x_pow * x) % MOD;
  }

  return val;
}

void swapRows(size_t cols, int32_t* matrix, size_t swap_row_1, size_t swap_row_2) {
  uint32_t col_byte_size = cols * sizeof(matrix[0]);
  uint8_t aux[col_byte_size];
  memcpy(aux, &matrix[swap_row_1 * cols], col_byte_size);
  memcpy(&matrix[swap_row_1 * cols], &matrix[swap_row_2 * cols], col_byte_size);
  memcpy(&matrix[swap_row_2 * cols], aux, col_byte_size);
}

// TODO: remove
void printMatrix(uint32_t rows, uint32_t cols, int32_t* matrix) {
  for (uint32_t i = 0; i < rows; ++i) {
    for (uint32_t j = 0; j < cols; ++j) {
      printf("%3d ", matrix[(i * cols) + j]);
    }
    puts("");
  }
}

void printMatrix_uint8_t(uint32_t rows, uint32_t cols, uint8_t* matrix) {
  for (uint32_t i = 0; i < rows; ++i) {
    for (uint32_t j = 0; j < cols; ++j) {
      printf("%3d ", matrix[(i * cols) + j]);
    }
    puts("");
  }
}

// 0 is added only as padding to get correct indeces for the rest.
const int32_t inverseMod257[] = {
  0,   1,   129, 86,  193, 103, 43,  147, 225, 200, 180, 187, 150, 178, 202, 120, 241, 121, 100, 230, 90,  49,
  222, 190, 75,  72,  89,  238, 101, 195, 60,  199, 249, 148, 189, 235, 50,  132, 115, 145, 45,  163, 153, 6,
  111, 40,  95,  175, 166, 21,  36,  126, 173, 97,  119, 243, 179, 248, 226, 61,  30,  59,  228, 102, 253, 87,
  74,  234, 223, 149, 246, 181, 25,  169, 66,  24,  186, 247, 201, 244, 151, 165, 210, 96,  205, 127, 3,   65,
  184, 26,  20,  209, 176, 152, 216, 46,  83,  53,  139, 135, 18,  28,  63,  5,   215, 164, 177, 245, 188, 224,
  250, 44,  218, 116, 124, 38,  113, 134, 159, 54,  15,  17,  158, 140, 114, 220, 51,  85,  255, 2,   172, 206,
  37,  143, 117, 99,  240, 242, 203, 98,  123, 144, 219, 133, 141, 39,  213, 7,   33,  69,  12,  80,  93,  42,
  252, 194, 229, 239, 122, 118, 204, 174, 211, 41,  105, 81,  48,  237, 231, 73,  192, 254, 130, 52,  161, 47,
  92,  106, 13,  56,  10,  71,  233, 191, 88,  232, 76,  11,  108, 34,  23,  183, 170, 4,   155, 29,  198, 227,
  196, 31,  9,   78,  14,  138, 160, 84,  131, 221, 236, 91,  82,  162, 217, 146, 251, 104, 94,  212, 112, 142,
  125, 207, 22,  68,  109, 8,   58,  197, 62,  156, 19,  168, 185, 182, 67,  35,  208, 167, 27,  157, 136, 16,
  137, 55,  79,  107, 70,  77,  57,  32,  110, 214, 154, 64,  171, 128,
};

// Adapted from: https://en.wikipedia.org/wiki/Gaussian_elimination#Pseudocode
void gaussEliminationModulo(uint32_t rows, uint32_t cols, int32_t* matrix) {
  uint32_t row_pivot = 0;
  uint32_t col_pivot = 0;
  // Casting int32_t* to pointer of int32_t[cols] which allows use of double subscript
  // element access.
  int32_t (*m)[cols] = (int32_t (*)[cols])matrix;

  while (row_pivot < rows && col_pivot < cols) {
    // Find the k-th pivot.
    uint32_t i_max = row_pivot;
    uint8_t max = m[i_max][col_pivot];
    for (uint32_t i = i_max + 1; i < rows; ++i) {
      if (abs(m[i][col_pivot]) > max) {
        max = m[i][col_pivot];
        i_max = i;
      }
    }
    // No pivot in this column, pass to next column.
    if (m[i_max][col_pivot] == 0) ++col_pivot;
    else {
      swapRows(cols, matrix, row_pivot, i_max);
      // Do for all rows below pivot.
      for (uint32_t i = row_pivot + 1; i < rows; ++i) {
        int32_t val = m[row_pivot][col_pivot];
        int32_t factor = m[i][col_pivot] * inverseMod257[val];
        // Fill with zeros the lower part of pivot column.
        m[i][col_pivot] = 0;
        // Do for all remaining elements in current row.
        for (uint32_t j = col_pivot + 1; j < cols; ++j) {
          m[i][j] = (m[i][j] - m[row_pivot][j] * factor) % MOD;
          if (m[i][j] < 0) m[i][j] += MOD;
        }
      }
      ++row_pivot;
      ++col_pivot;
    }
  }
}

void solveSystem(uint32_t rows, uint32_t cols, int32_t* matrix, uint8_t* coeficients) {
  if (cols != rows + 1) {
    fprintf(stderr, "Invalid matrix dimensions: %d x %d", rows, cols);
    exit(EXIT_FAILURE);
  }
  gaussEliminationModulo(rows, cols, matrix);
  int32_t (*m)[cols] = (int32_t (*)[cols])matrix;
  for (int64_t i = rows - 1; i >= 0; --i) {
    int32_t coef = m[i][cols - 1];
    for (uint32_t j = cols - 2; j >= i + 1; --j) {
      coef = (coef - m[i][j] * coeficients[j]) % MOD;
    }
    coef = (coef * inverseMod257[m[i][i]]) % MOD;
    if (coef < 0) coef += MOD;
    coeficients[i] = coef;
  }
}
