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
  uint32_t width, uint32_t height, uint16_t bpp, u_char reserved[4], uint32_t n_colors,
  Color colors[n_colors], uint32_t extra_data_size, u_char extra_data[extra_data_size]
);
BMP bmpParse(const char* filename);
void bmpFree(BMP bmp);
u_char* bmpImage(BMP bmp);
uint32_t bmpImageSize(BMP bmp);
uint32_t bmpWidth(BMP bmp);
uint32_t bmpHeight(BMP bmp);
uint32_t bmpBpp(BMP bmp);
uint32_t bmpNColors(BMP bmp);
Color* bmpColors(BMP bmp);
uint32_t bmpExtraSize(BMP bmp);
u_char* bmpExtraData(BMP bmp);
int bmpWriteFile(const char* filename, BMP bmp);
void bmpPrintHeader(BMP bmp);

#endif
