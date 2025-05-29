#include <stdint.h>

static uint64_t _seed;

void setSeed(uint64_t seed) {
  _seed = (seed ^ 0x5DEECE66Dlu) & ((1llu << 48u) - 1);
}

uint8_t nextChar() {
  _seed = (_seed * 0x5DEECE66DL + 0xBL) & ((1llu << 48u) - 1);
  return (uint8_t)(_seed >> 40u);
}

void permutationMatrix(uint32_t size, uint8_t* matrix) {
  for (uint32_t i = 0; i < size; ++i) {
    matrix[i] = nextChar();
  }
}

void xorMatrixes(uint32_t size, uint8_t* dest, const uint8_t* other) {
  for (uint32_t i = 0; i < size; ++i) dest[i] ^= other[i];

  // TODO: Try this and test that it works in pampero.
  // This uses AVX2 (Advanced Vector Extensions 2) to process 32 bytes in a single instruction!
  // size_t i = 0;
  // for (; i + 64 <= size; i += 64) {
  //     __m512i va = _mm512_loadu_si512((__m512i*)(a + i));
  //     __m512i vb = _mm512_loadu_si512((__m512i*)(b + i));
  //     __m512i vx = _mm512_xor_si512(va, vb);
  //     _mm512_storeu_si512((__m512i*)(dst + i), vx);
  // }
  // for (; i < size; i++) {
  //     dst[i] = a[i] ^ b[i];
  // }
}
