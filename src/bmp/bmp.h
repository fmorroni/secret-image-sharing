#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct BMP_CDT* BMP;

typedef struct Color {
  uint8_t b; // Blue
  uint8_t g; // Green
  uint8_t r; // Red
  uint8_t f; // Filler
} Color;

BMP bmpNew(
  uint32_t width, uint32_t height, uint16_t bpp, uint8_t reserved[4], uint32_t n_colors,
  Color colors[n_colors], uint32_t extra_data_size, uint8_t extra_data[extra_data_size]
);
BMP bmpParse(const char* filename);
void bmpFree(BMP bmp);
uint8_t* bmpImage(BMP bmp);
uint32_t bmpImageSize(BMP bmp);
uint32_t bmpWidth(BMP bmp);
uint32_t bmpHeight(BMP bmp);
uint32_t bmpBpp(BMP bmp);
uint32_t bmpNColors(BMP bmp);
Color* bmpColors(BMP bmp);
uint32_t bmpExtraSize(BMP bmp);
uint8_t* bmpExtraData(BMP bmp);
void bmpSetExtraData(BMP bmp, uint32_t extra_data_size, uint8_t* extra_data);
uint8_t* bmpReserved(BMP bmp);
void bmpSetReserved(BMP bmp, uint8_t reserved[4]);
int bmpWriteFile(const char* filename, BMP bmp);
void bmpPrintHeader(BMP bmp);

#endif
