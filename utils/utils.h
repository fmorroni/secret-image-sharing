#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <sys/types.h>

uint32_t ceilDiv(uint32_t numerator, uint32_t denominator);
void closestDivisors(uint32_t size, uint32_t* rows_out, uint32_t* cols_out);
uint32_t polynomialModuloEval(u_char order, const u_char coefficients[], u_char x);
void gaussEliminationModulo(uint32_t rows, uint32_t cols, int32_t* matrix);
void solveSystem(uint32_t rows, uint32_t cols, int32_t* matrix, u_char* coeficients);

// TODO: remove
void printMatrix(uint32_t rows, uint32_t cols, int32_t* matrix);
void printMatrix_u_char(uint32_t rows, uint32_t cols, u_char* matrix);

#endif
