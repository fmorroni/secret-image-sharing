#include "sis.h"
#include "../bmp/bmp.h"
#include "../globals.h"
#include "../utils/utils.h"
#include "permutation.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t bpp;
  uint32_t n_colors;
  Color colors[];
} ExtraData;

void calculateShadowPixel(
  uint8_t min_shadows, uint8_t coefficients[min_shadows], uint8_t tot_shadows, uint32_t pixels[tot_shadows]
);
void hideShadowPixels(
  uint32_t shadow_pixel_idx, uint8_t* coefficients, uint8_t min_shadows, uint8_t tot_shadows,
  BMP carrier_bmps[tot_shadows]
);
void stegHidePixel(uint32_t shadow_pixel_idx, uint8_t* img, uint8_t hide_pixel);
uint8_t stegRecoverPixel(uint32_t shadow_pixel_idx, uint8_t* img);
void writeExtraData(BMP bmp, uint8_t* extra_data);
void readExtraData(uint8_t* extra_data_raw, ExtraData** extra_data);

void sisShadows(BMP bmp, uint8_t min_shadows, uint8_t tot_shadows, BMP carrier_bmps[tot_shadows], uint16_t seed) {
  assert(min_shadows >= 2 && tot_shadows >= min_shadows);
  const uint8_t* img = bmpImage(bmp);
  uint32_t img_size = bmpImageSize(bmp);
  uint32_t shadow_size = ceilDiv(img_size, min_shadows);

  for (int i = 0; i < tot_shadows; ++i) {
    uint32_t carrier_size = bmpImageSize(carrier_bmps[i]);
    if (carrier_size < 8 * shadow_size) {
      fprintf(
        stderr,
        "sisShadows: Carrier image size must be at least 8x bigger than shadow size in order to hide the shadows "
        "(carrier_size %u < 8 x shadow_size %u)",
        carrier_size, 8 * shadow_size
      );
      exit(EXIT_FAILURE);
    }
  }

  setSeed(seed);
  uint8_t permMat[img_size];
  permutationMatrix(img_size, permMat);
  xorMatrixes(img_size, permMat, img);

  uint8_t seed_low = seed & 0xFFu;
  uint8_t seed_high = ((uint32_t)seed >> 8u) & 0xFFu;

  uint32_t extra_data_size = (4 * sizeof(uint32_t)) + (bmpNColors(bmp) * sizeof(Color));
  uint8_t extra_data[extra_data_size];
  writeExtraData(bmp, extra_data);

  for (uint8_t i = 0; i < tot_shadows; ++i) {
    bmpSetReserved(carrier_bmps[i], (uint8_t[]){seed_low, seed_high, i + 1, 0});
    bmpSetExtraData(carrier_bmps[i], extra_data_size, extra_data);
  }

  uint8_t coefficients[min_shadows];
  uint32_t i;
  for (i = 0; i < img_size / min_shadows; ++i) {
    for (int j = 0; j < min_shadows; ++j) coefficients[j] = permMat[(i * min_shadows) + j];
    hideShadowPixels(i, coefficients, min_shadows, tot_shadows, carrier_bmps);
  }

  // If img_size not multiple of r then use last img_size%r pixels, pad with
  // zeros and calculate a new shadow pixel.
  if (img_size % min_shadows != 0) {
    int j;
    for (i = i * min_shadows, j = 0; i < img_size; ++i, ++j) coefficients[j] = permMat[i];
    while (j < min_shadows) coefficients[j++] = 0;
    hideShadowPixels(shadow_size - 1, coefficients, min_shadows, tot_shadows, carrier_bmps);
  }
}

BMP sisRecover(uint8_t min_shadows, BMP shadows[min_shadows], uint16_t seed) {
  assert(min_shadows >= 2);
  uint32_t extra_data_size = bmpExtraSize(shadows[0]);
  if (extra_data_size < 3 * sizeof(uint32_t)) {
    fprintf(stderr, "Incorrect shadow format. Missing secret image info.\n");
    exit(EXIT_FAILURE);
  }
  uint16_t* reserved;
  uint16_t shadows_x[min_shadows];

  // This value is here to remove the possibility of a buffer overflow in case an incorrect
  // min_shadows value is used. That way you get a noise image in the output instead of an error.
  uint32_t max_valid_shadow_idx = UINT32_MAX;
  for (uint32_t i = 0; i < min_shadows; ++i) {
    uint32_t shadow_size = bmpImageSize(shadows[i]) - 8;
    uint32_t valid_k = shadow_size / 8;
    if (valid_k < max_valid_shadow_idx) {
      max_valid_shadow_idx = valid_k;
    }

    reserved = (uint16_t*)bmpReserved(shadows[i]);
    shadows_x[i] = reserved[1];
  }
  if (seed == 0) seed = reserved[0];

  // uint8_t* secret_info = bmpExtraData(shadows[0]);
  ExtraData* secret_info;
  readExtraData(bmpExtraData(shadows[0]), &secret_info);
  BMP secret = bmpNew(
    secret_info->width, secret_info->height, secret_info->bpp, NULL, secret_info->n_colors, secret_info->colors, 0, NULL
  );
  if (!secret) exit(EXIT_FAILURE);
  uint8_t* img = bmpImage(secret);
  uint32_t img_size = bmpImageSize(secret);
  uint32_t shadow_size = ceilDiv(img_size, min_shadows);
  uint32_t img_idx = 0;

  for (int i = 0; i < min_shadows; ++i) {
  }

  uint32_t safe_shadow_size = (shadow_size < max_valid_shadow_idx) ? shadow_size : max_valid_shadow_idx;

  for (uint32_t k = 0; k < safe_shadow_size; ++k) {
    uint32_t ec_system[min_shadows][min_shadows + 1];
    for (int i = 0; i < min_shadows; ++i) {
      uint32_t x_pow = 1;
      for (int j = 0; j < min_shadows; ++j) {
        ec_system[i][j] = x_pow;
        x_pow = (x_pow * shadows_x[i]) % MOD;
      }
      ec_system[i][min_shadows] = stegRecoverPixel(k, bmpImage(shadows[i]));
    }
    uint8_t coefs[min_shadows];
    solveSystem(min_shadows, min_shadows + 1, ec_system[0], coefs);
    for (int i = 0; i < min_shadows && img_idx < img_size; ++i, ++img_idx) {
      img[img_idx] = coefs[i];
    }
  }

  setSeed(seed);
  uint8_t permMat[img_size];
  permutationMatrix(img_size, permMat);
  xorMatrixes(img_size, img, permMat);

  return secret;
}

/*
   Img 5x3 con SIS (4,5). Un pixel de padding para d en 0

   a a a
   a b b      s1     s2     s3     s4    s5
   b b c ==> abcd   abcd   abcd   abcd  abcd
   c c c
   d d d

                i != j != k != l
   a_0 + a_1 * i + a_2 * i^2 + a_3 * i^3 = a_si
   a_0 + a_1 * j + a_2 * j^2 + a_3 * j^3 = a_sj
   a_0 + a_1 * k + a_2 * k^2 + a_3 * k^3 = a_sk
   a_0 + a_1 * l + a_2 * l^2 + a_3 * l^3 = a_sl

   b_0 + b_1 * i + b_2 * i^2 + b_3 * i^3 = b_si
   b_0 + b_1 * j + b_2 * j^2 + b_3 * j^3 = b_sj
   b_0 + b_1 * k + b_2 * k^2 + b_3 * k^3 = b_sk
   b_0 + b_1 * l + b_2 * l^2 + b_3 * l^3 = b_sl

   c_0 + c_1 * i + c_2 * i^2 + c_3 * i^3 = c_si
   c_0 + c_1 * j + c_2 * j^2 + c_3 * j^3 = c_sj
   c_0 + c_1 * k + c_2 * k^2 + c_3 * k^3 = c_sk
   c_0 + c_1 * l + c_2 * l^2 + c_3 * l^3 = c_sl

   d_0 + d_1 * i + d_2 * i^2 +   0 * i^3 = d_si
   d_0 + d_1 * j + d_2 * j^2 +   0 * j^3 = d_sj
   d_0 + d_1 * k + d_2 * k^2 +   0 * k^3 = d_sk
   d_0 + d_1 * l + d_2 * l^2 +   0 * l^3 = d_sl
*/

// Internal functions

void calculateShadowPixel(
  uint8_t min_shadows, uint8_t coefficients[min_shadows], uint8_t tot_shadows, uint32_t pixels[tot_shadows]
) {
  bool recalculate;
  do {
    recalculate = false;
    for (int i = 0; i < tot_shadows; ++i) pixels[i] = polynomialModuloEval(min_shadows - 1, coefficients, i + 1);
    for (int i = 0; i < tot_shadows; ++i) {
      if (pixels[i] == 256) {
        int j = 0;
        while (j < min_shadows && coefficients[j] == 0) ++j;
        assert(j < min_shadows && "Expected at least one non-zero coefficient");
        --coefficients[j];
        recalculate = true;
        break;
      }
    }
  } while (recalculate);
}

void hideShadowPixels(
  uint32_t shadow_pixel_idx, uint8_t* coefficients, uint8_t min_shadows, uint8_t tot_shadows,
  BMP carrier_bmps[tot_shadows]
) {
  uint32_t pixels[tot_shadows];
  calculateShadowPixel(min_shadows, coefficients, tot_shadows, pixels);
  for (int j = 0; j < tot_shadows; ++j) {
    uint8_t hide_pixel = pixels[j];
    stegHidePixel(shadow_pixel_idx, bmpImage(carrier_bmps[j]), hide_pixel);
  }
}

void stegHidePixel(uint32_t shadow_pixel_idx, uint8_t* img, uint8_t hide_pixel) {
  uint8_t hide_bits[8] = {
    hide_pixel >> 7u,           (hide_pixel & 0x40u) >> 6u, (hide_pixel & 0x20u) >> 5u, (hide_pixel & 0x10u) >> 4u,
    (hide_pixel & 0x08u) >> 3u, (hide_pixel & 0x04u) >> 2u, (hide_pixel & 0x02u) >> 1u, (hide_pixel & 0x01u) >> 0u,
  };
  uint8_t* offset_img = img + ((size_t)shadow_pixel_idx * 8);
  for (int j = 0; j < 8; ++j) offset_img[j] = (offset_img[j] & 0xFEu) | hide_bits[j];
}

uint8_t stegRecoverPixel(uint32_t shadow_pixel_idx, uint8_t* img) {
  uint8_t recoveredPixel = 0;
  uint8_t* offset_img = img + ((size_t)shadow_pixel_idx * 8);
  // printf("offset: %u\n", shadow_pixel_idx * 8);
  for (int j = 0; j < 8; ++j) {
    recoveredPixel <<= 1u;
    recoveredPixel |= offset_img[j] & 1u;
  }
  return recoveredPixel;
}

void writeExtraData(BMP bmp, uint8_t* extra_data) {
  ExtraData* extra_data_struct = (ExtraData*)extra_data;
  extra_data_struct->width = bmpWidth(bmp);
  extra_data_struct->height = bmpHeight(bmp);
  extra_data_struct->bpp = bmpBpp(bmp);
  extra_data_struct->n_colors = bmpNColors(bmp);
  memcpy(&extra_data_struct->colors, bmpColors(bmp), bmpNColors(bmp) * sizeof(Color));
}

void readExtraData(uint8_t* extra_data_raw, ExtraData** extra_data) {
  *extra_data = (ExtraData*)extra_data_raw;
}
