#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>

typedef struct BMP_HEADER_CDT* BMP_HEADER;

BMP_HEADER bmpParseHeader(FILE* bmp);
void bmpFreeHeader(BMP_HEADER header);
int32_t bmpImageSize(BMP_HEADER header);
int32_t bmpWidth(BMP_HEADER header);
int32_t bmpHeight(BMP_HEADER header);
int32_t bmpBpp(BMP_HEADER header);
int bmpParseBody(FILE* bmp, BMP_HEADER header, unsigned char* img);
int bmpWriteFile(FILE* file, BMP_HEADER header, unsigned char* img);
void bmpPrintHeader(BMP_HEADER header);

#endif
