#include "bmp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BMP_PARSE_CLEANUP(msg, bmp, file)                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    bmpFree(bmp);                                                                                  \
    fclose(file);                                                                                  \
    return NULL;                                                                                   \
  } while (0)

#define BMP_WRITE_CLEANUP(msg, file)                                                               \
  do {                                                                                             \
    perror(msg);                                                                                   \
    fclose(file);                                                                                  \
    return EXIT_FAILURE;                                                                           \
  } while (0)

typedef struct Color {
  unsigned char b; // Blue
  unsigned char g; // Green
  unsigned char r; // Red
  unsigned char f; // Filler
} Color;

typedef struct BMP_CDT {
  char id[2];
  int32_t filesize;             // In bytes.
  unsigned char reserved[4];    //
  int32_t offset;               //
  int32_t infoHeaderSize;       // Info header starts starts at this address.
  int32_t width;                // In pixels.
  int32_t height;               // In pixels.
  int16_t nPlanes;              // No clue what this is...
  int16_t bpp;                  // Bits Per Pixel.
  int32_t compressionType;      // 0: none - 1: RLE 8-bit/pixel - 2: RLE 4-bit/pixel - ...
  int32_t imageSize;            // In bytes.
  int32_t horizontalResolution; //
  int32_t verticalResolution;   //
  int32_t nColors;              // Number of colors.
  int32_t nImportantColors;     // Number of important colors. (???)
  Color* colors;
  unsigned char* image;
} BMP_CDT;

BMP bmpParse(const char* filename) {
  // "rb" is for binary files.
  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    perror("fopen");
    return NULL;
  }

  BMP bmp = malloc(sizeof(BMP_CDT));
  if (bmp == NULL) {
    perror("malloc");
    return NULL;
  }

  // I need to split the reading because in the strcut, the `filesize` field
  // will be shifted 2 bytes to align with the closest dword.
  size_t read = fread(bmp, 2, 1, file);
  if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);

  // This reads the default header upto infoHeaderSize.
  read = fread(&bmp->filesize, 16, 1, file);
  if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);

  // The size specified in infoHeaderSize includes itself so we subtruct its size.
  read = fread(&bmp->width, bmp->infoHeaderSize - 4, 1, file);
  if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);

  if (bmp->nImportantColors != 0) {
    fprintf(stderr, "Number of important colors != 0. Idk what to do, look it up...");
  }

  // Available colors are sometimes (not sure why not always...) defined in the header,
  // in a 4 byte format for B-G-R-Filler.
  if (bmp->nColors > 0) {
    size_t colorBytes = sizeof(Color) * bmp->nColors;
    bmp->colors = malloc(colorBytes);
    if (bmp->colors == NULL) BMP_PARSE_CLEANUP("malloc", bmp, file);

    read = fread(bmp->colors, colorBytes, 1, file);
    if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);
  }

  bmp->image = malloc(bmp->imageSize);
  if (bmp->image == NULL) BMP_PARSE_CLEANUP("malloc", bmp, file);

  if (fseek(file, bmp->offset, SEEK_SET) != 0) BMP_PARSE_CLEANUP("fseek", bmp, file);
  read = fread(bmp->image, 1, bmp->imageSize, file);
  if (read != bmp->imageSize) BMP_PARSE_CLEANUP("fread", bmp, file);

  if (fclose(file) != 0) {
    perror("fclose");
    bmpFree(bmp);
    return NULL;
  }
  return bmp;
}

void bmpFree(BMP header) {
  if (header != NULL) {
    if (header->colors != NULL) {
      free(header->colors);
    }
    if (header->image != NULL) {
      free(header->image);
    }
    free(header);
  }
}

unsigned char* bmpImage(BMP bmp) {
  return bmp->image;
}

int32_t bmpImageSize(BMP bmp) {
  return bmp->imageSize;
}

int32_t bmpWidth(BMP bmp) {
  return bmp->width;
}

int32_t bmpHeight(BMP bmp) {
  return bmp->height;
}

int32_t bmpBpp(BMP bmp) {
  return bmp->bpp;
}

int bmpWriteFile(const char* filename, BMP bmp) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  size_t written = fwrite(bmp, 2, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  written = fwrite(&bmp->filesize, 16, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  written = fwrite(&bmp->width, bmp->infoHeaderSize - 4, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  if (bmp->nColors > 0) {
    written = fwrite(bmp->colors, sizeof(Color) * bmp->nColors, 1, file);
    if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);
  }

  if (fseek(file, bmp->offset, SEEK_SET) != 0) BMP_WRITE_CLEANUP("fseek", file);
  written = fwrite(bmp->image, bmp->imageSize, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  return 0;
}

void printColor(Color c) {
  printf("#%02x%02x%02x", c.r, c.g, c.b);
}

void bmpPrintHeader(BMP bmp) {
  printf("=== BMP Header ===\n");
  printf("ID:                 %c%c\n", bmp->id[0], bmp->id[1]);
  printf("Filesize:           %d bytes\n", bmp->filesize);
  printf(
    "Reserved:           %02x %02x %02x %02x\n", bmp->reserved[0], bmp->reserved[1],
    bmp->reserved[2], bmp->reserved[3]
  );
  printf("Pixel Data Offset:  %d bytes\n", bmp->offset);
  printf("Info Header Size:   %d bytes\n", bmp->infoHeaderSize);
  printf("Width:              %d px\n", bmp->width);
  printf("Height:             %d px\n", bmp->height);
  printf("Planes:             %d\n", bmp->nPlanes);
  printf("Bits Per Pixel:     %d\n", bmp->bpp);
  printf("Compression Type:   %d\n", bmp->compressionType);
  printf("Image Size:         %d bytes\n", bmp->imageSize);
  printf("X Resolution:       %d px/meter\n", bmp->horizontalResolution);
  printf("Y Resolution:       %d px/meter\n", bmp->verticalResolution);
  printf("Total Colors:       %d\n", bmp->nColors);
  printf("Important Colors:   %d\n", bmp->nImportantColors);
  if (bmp->nColors > 0) {
    printf("Colors: [ ");
    for (int i = 0; i < bmp->nColors; ++i) {
      printColor(bmp->colors[i]);
      if (i < bmp->nColors - 1) printf(", ");
    }
    printf("]\n");
  }
}
