#include "bmp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Color {
  unsigned char b; // Blue
  unsigned char g; // Green
  unsigned char r; // Red
  unsigned char f; // Filler
} Color;

typedef struct BMP_HEADER_CDT {
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
} BMP_HEADER_CDT;

BMP_HEADER bmpParseHeader(FILE* bmp) {
  BMP_HEADER header = malloc(sizeof(BMP_HEADER_CDT));
  if (header == NULL) return NULL;

  // I need to split the reading because in the strcut, the `filesize` field
  // will be shifted 2 bytes to align with the closest dword.
  size_t read = fread(header, 2, 1, bmp);
  if (read != 1) {
    perror("fread");
    return NULL;
  }

  // This reads the default header upto infoHeaderSize.
  read = fread(&header->filesize, 16, 1, bmp);
  if (read != 1) {
    perror("fread");
    return NULL;
  }

  // The size specified in infoHeaderSize includes itself so we subtruct its size.
  read = fread(&header->width, header->infoHeaderSize - 4, 1, bmp);
  if (read != 1) {
    perror("fread");
    return NULL;
  }

  if (header->nImportantColors != 0) {
    fprintf(stderr, "Number of important colors != 0. Idk what to do, look it up...");
  }

  // Available colors are sometimes (not sure why not always...) defined in the header,
  // in a 4 byte format for B-G-R-Filler.
  if (header->nColors > 0) {
    size_t colorBytes = sizeof(Color) * header->nColors;
    header->colors = malloc(colorBytes);

    read = fread(header->colors, colorBytes, 1, bmp);
    if (read != 1) {
      perror("fread");
      return NULL;
    }
  }

  return header;
}

void bmpFreeHeader(BMP_HEADER header) {
  if (header->nColors > 0) {
    free(header->colors);
  }
  free(header);
}

int32_t bmpImageSize(BMP_HEADER header) {
  return header->imageSize;
}

int32_t bmpWidth(BMP_HEADER header) {
  return header->width;
}

int32_t bmpHeight(BMP_HEADER header) {
  return header->height;
}

int32_t bmpBpp(BMP_HEADER header) {
  return header->bpp;
}

int bmpParseBody(FILE* bmp, BMP_HEADER header, unsigned char* img) {
  if (fseek(bmp, header->offset, SEEK_SET) != 0) {
    perror("fseek");
    return 1;
  }
  size_t read = fread(img, 1, header->imageSize, bmp);
  if (read != header->imageSize) {
    perror("fread");
    return 1;
  }
  return 0;
}

int bmpWriteFile(FILE* file, BMP_HEADER header, unsigned char* img) {
  size_t written = fwrite(header, 2, 1, file);
  if (written != 1) {
    perror("fwrite");
    return 1;
  }
  written = fwrite(&header->filesize, 16, 1, file);
  if (written != 1) {
    perror("fwrite");
    return 1;
  }
  written = fwrite(&header->width, header->infoHeaderSize - 4, 1, file);
  if (written != 1) {
    perror("fwrite");
    return 1;
  }
  if (header->nColors > 0) {
    written = fwrite(header->colors, sizeof(Color) * header->nColors, 1, file);
    if (written != 1) {
      perror("fwrite");
      return 1;
    }
  }

  if (fseek(file, header->offset, SEEK_SET) != 0) {
    perror("fseek");
    return 1;
  }
  written = fwrite(img, header->imageSize, 1, file);
  if (written != 1) {
    perror("fwrite");
    return 1;
  }

  return 0;
}

void printColor(Color c) {
  printf("#%02x%02x%02x", c.r, c.g, c.b);
}

void bmpPrintHeader(BMP_HEADER header) {
  printf("=== BMP Header ===\n");
  printf("ID:                 %c%c\n", header->id[0], header->id[1]);
  printf("Filesize:           %d bytes\n", header->filesize);
  printf(
    "Reserved:           %02x %02x %02x %02x\n", header->reserved[0], header->reserved[1],
    header->reserved[2], header->reserved[3]
  );
  printf("Pixel Data Offset:  %d bytes\n", header->offset);
  printf("Info Header Size:   %d bytes\n", header->infoHeaderSize);
  printf("Width:              %d px\n", header->width);
  printf("Height:             %d px\n", header->height);
  printf("Planes:             %d\n", header->nPlanes);
  printf("Bits Per Pixel:     %d\n", header->bpp);
  printf("Compression Type:   %d\n", header->compressionType);
  printf("Image Size:         %d bytes\n", header->imageSize);
  printf("X Resolution:       %d px/meter\n", header->horizontalResolution);
  printf("Y Resolution:       %d px/meter\n", header->verticalResolution);
  printf("Total Colors:       %d\n", header->nColors);
  printf("Important Colors:   %d\n", header->nImportantColors);
  if (header->nColors > 0) {
    printf("Colors: [ ");
    for (int i = 0; i < header->nColors; ++i) {
      printColor(header->colors[i]);
      if (i < header->nColors - 1) printf(", ");
    }
    printf("]\n");
  }
}
