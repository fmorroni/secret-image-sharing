#include "bmp.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BMP_SIMPLE_CLEANUP(msg, bmp)                                                               \
  do {                                                                                             \
    perror(msg);                                                                                   \
    bmpFree(bmp);                                                                                  \
    return NULL;                                                                                   \
  } while (0)

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

typedef struct BMP_CDT {
  char id[2];
  int32_t filesize;              // In bytes.
  u_char reserved[4];            //
  int32_t offset;                //
  int32_t info_header_size;      // Info header starts starts at this address.
  int32_t width;                 // In pixels.
  int32_t height;                // In pixels.
  int16_t n_planes;              // No clue what this is...
  int16_t bpp;                   // Bits Per Pixel.
  int32_t compression_type;      // 0: none - 1: RLE 8-bit/pixel - 2: RLE 4-bit/pixel - ...
  int32_t image_size;            // In bytes.
  int32_t horizontal_resolution; //
  int32_t vertical_resolution;   //
  int32_t n_colors;              // Number of colors.
  int32_t n_important_colors;    // Number of important colors. (???)
  Color* colors;
  u_char* image;
} BMP_CDT;

BMP bmpNew(
  int32_t width, int32_t height, int16_t bpp, u_char reserved[4], int32_t n_colors,
  Color colors[n_colors]
) {
  if (bpp % 8 != 0) {
    errno = EINVAL;
    return NULL;
  }

  BMP bmp = malloc(sizeof(BMP_CDT));
  if (bmp == NULL) {
    perror("malloc");
    return NULL;
  }

  int32_t image_size = width * height * bpp / 8;
  int32_t header_size = 54 + sizeof(Color) * n_colors;

  bmp->id[0] = 'B';
  bmp->id[1] = 'M';
  bmp->filesize = width * height * bpp / 8 + header_size;
  for (int i = 0; i < 4; ++i) bmp->reserved[i] = reserved[i];
  bmp->offset = header_size;
  bmp->info_header_size = 40;
  bmp->width = width;
  bmp->height = height;
  bmp->n_planes = 1;
  bmp->bpp = bpp;
  bmp->compression_type = 0;
  bmp->image_size = image_size;
  bmp->horizontal_resolution = 0;
  bmp->vertical_resolution = 0;
  bmp->n_colors = n_colors;
  bmp->n_important_colors = 0;
  if (n_colors > 0 && colors != NULL) {
    size_t color_bytes = sizeof(Color) * bmp->n_colors;
    bmp->colors = malloc(color_bytes);
    if (bmp->colors == NULL) BMP_SIMPLE_CLEANUP("malloc", bmp);
    memcpy(bmp->colors, colors, color_bytes);
  }
  bmp->image = malloc(image_size);
  if (bmp->image == NULL) BMP_SIMPLE_CLEANUP("malloc", bmp);

  return bmp;
}

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

  // This reads the default header upto info_header_size.
  read = fread(&bmp->filesize, 16, 1, file);
  if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);

  // The size specified in info_header_size includes itself so we subtruct its size.
  read = fread(&bmp->width, bmp->info_header_size - 4, 1, file);
  if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);

  if (bmp->n_important_colors != 0) {
    fprintf(stderr, "Number of important colors != 0. Idk what to do, look it up...");
  }

  // Available colors are sometimes (not sure why not always...) defined in the header,
  // in a 4 byte format for B-G-R-Filler.
  if (bmp->n_colors > 0) {
    size_t color_bytes = sizeof(Color) * bmp->n_colors;
    bmp->colors = malloc(color_bytes);
    if (bmp->colors == NULL) BMP_PARSE_CLEANUP("malloc", bmp, file);

    read = fread(bmp->colors, color_bytes, 1, file);
    if (read != 1) BMP_PARSE_CLEANUP("fread", bmp, file);
  }

  bmp->image = malloc(bmp->image_size);
  if (bmp->image == NULL) BMP_PARSE_CLEANUP("malloc", bmp, file);

  if (fseek(file, bmp->offset, SEEK_SET) != 0) BMP_PARSE_CLEANUP("fseek", bmp, file);
  read = fread(bmp->image, 1, bmp->image_size, file);
  if (read != bmp->image_size) BMP_PARSE_CLEANUP("fread", bmp, file);

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

u_char* bmpImage(BMP bmp) {
  return bmp->image;
}

int32_t bmpImageSize(BMP bmp) {
  return bmp->image_size;
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

int32_t bmpNColors(BMP bmp) {
  return bmp->n_colors;
}

Color* bmpColors(BMP bmp) {
  return bmp->colors;
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

  written = fwrite(&bmp->width, bmp->info_header_size - 4, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  if (bmp->n_colors > 0) {
    written = fwrite(bmp->colors, sizeof(Color) * bmp->n_colors, 1, file);
    if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);
  }

  if (fseek(file, bmp->offset, SEEK_SET) != 0) BMP_WRITE_CLEANUP("fseek", file);
  written = fwrite(bmp->image, bmp->image_size, 1, file);
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
  printf("Info Header Size:   %d bytes\n", bmp->info_header_size);
  printf("Width:              %d px\n", bmp->width);
  printf("Height:             %d px\n", bmp->height);
  printf("Planes:             %d\n", bmp->n_planes);
  printf("Bits Per Pixel:     %d\n", bmp->bpp);
  printf("Compression Type:   %d\n", bmp->compression_type);
  printf("Image Size:         %d bytes\n", bmp->image_size);
  printf("X Resolution:       %d px/meter\n", bmp->horizontal_resolution);
  printf("Y Resolution:       %d px/meter\n", bmp->vertical_resolution);
  printf("Total Colors:       %d\n", bmp->n_colors);
  printf("Important Colors:   %d\n", bmp->n_important_colors);
  if (bmp->n_colors > 0) {
    printf("Colors: [ ");
    for (int i = 0; i < bmp->n_colors; ++i) {
      printColor(bmp->colors[i]);
      if (i < bmp->n_colors - 1) printf(", ");
    }
    printf("]\n");
  }
}
