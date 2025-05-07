#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

uint32_t ceilDiv(uint32_t a, uint32_t b) {
  if (b == 0) {
    fprintf(stderr, "Devided by zero, stupid... -> %d/%d", a, b);
    return -1;
  }
  return ((uint64_t)a + b - 1) / b;
}

void closestDivisors(uint32_t N, uint32_t* r_out, uint32_t* c_out) {
  uint32_t r, c;
  uint32_t best_r = 1, best_c = N;
  uint32_t min_diff = N;

  for (r = 1; r * r <= N; ++r) {
    if (N % r == 0) {
      c = N / r;
      uint32_t diff = (r > c) ? r - c : c - r;
      if (diff < min_diff) {
        min_diff = diff;
        best_r = r;
        best_c = c;
      }
    }
  }

  *r_out = best_r;
  *c_out = best_c;
}

int32_t polynomialModuloEval(u_char order, u_char coefficients[], int32_t mod, u_char x) {
  int32_t v = 0;

  u_char x_pow = 1;
  for (int i = 0; i < order + 1; ++i) {
    v += ((int32_t)coefficients[i] * x_pow);
    v %= mod;
    x_pow = (x_pow * x) % mod;
  }

  return v;
}
