#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct BMP_CDT* BMP;

typedef struct Color {
  u_char b; // Blue
  u_char g; // Green
  u_char r; // Red
  u_char f; // Filler
} Color;

BMP bmpNew(
  int32_t width, int32_t height, int16_t bpp, u_char reserved[4], int32_t n_colors,
  Color colors[n_colors]
);
BMP bmpParse(const char* filename);
void bmpFree(BMP bmp);
u_char* bmpImage(BMP bmp);
int32_t bmpImageSize(BMP bmp);
int32_t bmpWidth(BMP bmp);
int32_t bmpHeight(BMP bmp);
int32_t bmpBpp(BMP bmp);
int32_t bmpNColors(BMP bmp);
Color* bmpColors(BMP bmp);
int bmpWriteFile(const char* filename, BMP bmp);
void bmpPrintHeader(BMP bmp);

#endif
