#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <sys/types.h>

uint32_t ceilDiv(uint32_t a, uint32_t b);
void closestDivisors(uint32_t N, uint32_t* r_out, uint32_t* c_out);
int32_t polynomialModuloEval(u_char order, u_char coefficients[], u_char x);

#endif
