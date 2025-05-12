#include "sis.h"
#include "../bmp/bmp.h"
#include "../utils/utils.h"
#include <stdint.h>
#include <sys/types.h>

#define MOD 257

void sisShadows(BMP bmp, u_char r, u_char n) {
  const u_char* img = bmpImage(bmp);
  uint32_t img_size = bmpImageSize(bmp);

  // TODO: handle seed properly...
  uint16_t seed = 0x1234;
  u_char seed_low = seed & 0xFF;
  u_char seed_high = (seed >> 8) & 0xFF;
  //

  uint32_t shadow_size = ceilDiv(img_size, r);
  uint32_t shadow_rows, shadow_cols;
  closestDivisors(shadow_size, &shadow_rows, &shadow_cols);

  BMP shadows[n];
  for (u_char i = 0; i < n; ++i) {
    shadows[i] = bmpNew(
      shadow_cols, shadow_rows, bmpBpp(bmp), (u_char[]){seed_low, seed_high, i + 1, 0},
      bmpNColors(bmp), bmpColors(bmp), 0, NULL
    );
  }

  u_char coefficients[r];
  uint32_t i;
  for (i = 0; i < img_size / r; ++i) {
    for (int j = 0; j < r; ++j) {
      coefficients[j] = img[i * r + j];
    }
    for (int j = 0; j < n; ++j) {
      int32_t v = polynomialModuloEval(r - 1, coefficients, j + 1);

      // TODO: Handle properly.
      if (v == 256) v = 255;

      bmpImage(shadows[j])[i] = v;
    }
  }
  int j;
  for (i = i * r, j = 0; i < img_size; ++i, ++j) coefficients[j] = img[i];
  while (j < r) coefficients[j++] = 0;
  for (int j = 0; j < n; ++j) {
    int32_t v = polynomialModuloEval(r - 1, coefficients, j + 1);

    // TODO: Handle properly.
    if (v == 256) v = 255;

    bmpImage(shadows[j])[shadow_size - 1] = v;
  }

  for (int i = 0; i < n; ++i) {
    char name[20];
    snprintf(name, sizeof(name), "test-%02d.bmp", i + 1);
    bmpWriteFile(name, shadows[i]);
    bmpFree(shadows[i]);
  }
}
