#ifndef PERMUTATION_H
#define PERMUTATION_H

#include <stdint.h>

void setSeed(uint64_t seed);
void permutationMatrix(uint32_t size, uint8_t* matrix);
void xorMatrixes(uint32_t size, uint8_t* dest, const uint8_t* other);

#endif
