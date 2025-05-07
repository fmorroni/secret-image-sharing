#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>

typedef struct BMP_CDT* BMP;

BMP bmpParse(const char* filename);
void bmpFree(BMP bmp);
unsigned char* bmpImage(BMP bmp);
int32_t bmpImageSize(BMP bmp);
int32_t bmpWidth(BMP bmp);
int32_t bmpHeight(BMP bmp);
int32_t bmpBpp(BMP bmp);
int bmpWriteFile(const char* filename, BMP bmp);
void bmpPrintHeader(BMP bmp);

#endif
