#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

uint32_t ceilDiv(uint32_t numerator, uint32_t denominator);
void closestDivisors(uint32_t size, uint32_t* rows_out, uint32_t* cols_out);
uint32_t polynomialModuloEval(uint8_t order, const uint8_t coefficients[], uint8_t x);
void gaussEliminationModulo(uint32_t rows, uint32_t cols, int32_t* matrix);
void solveSystem(uint32_t rows, uint32_t cols, int32_t* matrix, uint8_t* coeficients);

// TODO: remove
void printMatrix(uint32_t rows, uint32_t cols, int32_t* matrix);
void printMatrix_uint8_t(uint32_t rows, uint32_t cols, uint8_t* matrix);

#endif
