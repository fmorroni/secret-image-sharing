#include "bmp.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMP_SIMPLE_CLEANUP(msg, bmp)                                                                                   \
  do {                                                                                                                 \
    perror(msg);                                                                                                       \
    bmpFree(bmp);                                                                                                      \
    return NULL;                                                                                                       \
  } while (0)

#define BMP_WRITE_CLEANUP(msg, file)                                                                                   \
  do {                                                                                                                 \
    perror(msg);                                                                                                       \
    fclose(file);                                                                                                      \
    return 1;                                                                                                          \
  } while (0)

#define BYTE_SIZE 8
#define BASE_HEADER_SIZE 14
#define DEFAULT_INFO_HEADER_SIZE 40

typedef struct BMP_CDT {
  char id[2];
  uint32_t filesize;              // In bytes.
  uint8_t reserved[4];            //
  uint32_t offset;                //
  uint32_t info_header_size;      // Info header starts starts at this address.
  uint32_t width;                 // In pixels.
  uint32_t height;                // In pixels.
  uint16_t n_planes;              // No clue what this is...
  uint16_t bpp;                   // Bits Per Pixel.
  uint32_t compression_type;      // 0: none - 1: RLE 8-bit/pixel - 2: RLE 4-bit/pixel - ...
  uint32_t image_size;            // In bytes.
  uint32_t horizontal_resolution; //
  uint32_t vertical_resolution;   //
  uint32_t n_colors;              // Number of colors.
  uint32_t n_important_colors;    // Number of important colors. (???)
  Color* colors;
  uint32_t extra_data_size;
  uint8_t* extra_data;
  uint8_t* image;
} BMP_CDT;

void printColor(Color color);
static bool freadWithPerror(FILE* file, void* dest, size_t size, const char* err);
static bool parseBaseHeader(FILE* file, BMP bmp);
static bool parseInfoHeader(FILE* file, BMP bmp);
static bool parseColorTable(FILE* file, BMP bmp);
static bool parseExtraData(FILE* file, BMP bmp);
static bool parseImageData(FILE* file, BMP bmp);

BMP bmpNew(
  uint32_t width, uint32_t height, uint16_t bpp, uint8_t reserved[4], uint32_t n_colors, Color colors[n_colors],
  uint32_t extra_data_size, uint8_t extra_data[extra_data_size]
) {
  if (bpp % BYTE_SIZE != 0) {
    errno = EINVAL;
    return NULL;
  }

  BMP bmp = malloc(sizeof(BMP_CDT));
  if (bmp == NULL) {
    perror("malloc");
    return NULL;
  }
  bmp->image = NULL;
  bmp->extra_data = NULL;

  uint32_t image_size = width * height * bpp / BYTE_SIZE;
  uint32_t extra_data_bytes = extra_data_size == 0 ? 0 : 4 + extra_data_size;
  uint32_t header_size = BASE_HEADER_SIZE + DEFAULT_INFO_HEADER_SIZE + (sizeof(Color) * n_colors) + extra_data_bytes;

  bmp->id[0] = 'B';
  bmp->id[1] = 'M';
  bmp->filesize = image_size + header_size;
  if (reserved != NULL) memcpy(bmp->reserved, reserved, 4);
  else memset(bmp->reserved, 0, 4);
  bmp->offset = header_size;
  bmp->info_header_size = DEFAULT_INFO_HEADER_SIZE;
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
  } else {
    bmp->n_colors = 0;
    bmp->colors = NULL;
  }
  if (extra_data_size > 0 && extra_data != NULL) {
    bmp->extra_data_size = extra_data_size;
    bmp->extra_data = malloc(extra_data_size);
    if (bmp->extra_data == NULL) BMP_SIMPLE_CLEANUP("malloc", bmp);
    memcpy(bmp->extra_data, extra_data, extra_data_size);
  } else {
    bmp->extra_data_size = 0;
    bmp->extra_data = NULL;
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
  bmp->colors = NULL;
  bmp->image = NULL;
  bmp->extra_data = NULL;
  if (bmp == NULL) {
    perror("malloc");
    fclose(file);
    return NULL;
  }

  if (!parseBaseHeader(file, bmp) || !parseInfoHeader(file, bmp) || !parseColorTable(file, bmp) ||
      !parseExtraData(file, bmp) || !parseImageData(file, bmp)) {
    bmpFree(bmp);
    fclose(file);
    return NULL;
  }

  if (bmp->n_important_colors != 0) {
    fprintf(stderr, "Number of important colors != 0. Idk what to do, look it up...");
  }

  if (fclose(file) != 0) {
    perror("fclose");
    bmpFree(bmp);
    return NULL;
  }
  return bmp;
}

void bmpFree(BMP bmp) {
  if (bmp != NULL) {
    if (bmp->colors != NULL) {
      free(bmp->colors);
    }
    if (bmp->image != NULL) {
      free(bmp->image);
    }
    if (bmp->extra_data != NULL) {
      free(bmp->extra_data);
    }
    free(bmp);
  }
}

uint8_t* bmpImage(BMP bmp) {
  return bmp->image;
}

uint32_t bmpImageSize(BMP bmp) {
  return bmp->image_size;
}

uint32_t bmpWidth(BMP bmp) {
  return bmp->width;
}

uint32_t bmpHeight(BMP bmp) {
  return bmp->height;
}

uint32_t bmpBpp(BMP bmp) {
  return bmp->bpp;
}

uint32_t bmpNColors(BMP bmp) {
  return bmp->n_colors;
}

Color* bmpColors(BMP bmp) {
  return bmp->colors;
}

uint32_t bmpExtraSize(BMP bmp) {
  return bmp->extra_data_size;
}

uint8_t* bmpExtraData(BMP bmp) {
  return bmp->extra_data;
}

uint8_t* bmpReserved(BMP bmp) {
  return bmp->reserved;
}

int bmpWriteFile(const char* filename, BMP bmp) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    perror("fopen");
    return 1;
  }

  size_t written = fwrite(bmp, 2, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  written = fwrite(&bmp->filesize, BASE_HEADER_SIZE - 2, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  written = fwrite(&bmp->info_header_size, bmp->info_header_size, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  if (bmp->n_colors > 0) {
    written = fwrite(bmp->colors, sizeof(Color) * bmp->n_colors, 1, file);
    if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);
  }

  if (bmp->extra_data_size > 0) {
    written = fwrite(&bmp->extra_data_size, 4, 1, file);
    if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);
    written = fwrite(bmp->extra_data, bmp->extra_data_size, 1, file);
    if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);
  }

  if (fseek(file, bmp->offset, SEEK_SET) != 0) BMP_WRITE_CLEANUP("fseek", file);
  written = fwrite(bmp->image, bmp->image_size, 1, file);
  if (written != 1) BMP_WRITE_CLEANUP("fwrite", file);

  if (fclose(file) != 0) {
    perror("fclose");
    return 1;
  }
  return 0;
}

void bmpPrintHeader(BMP bmp) {
  printf("=== BMP Header ===\n");
  printf("ID:                 %c%c\n", bmp->id[0], bmp->id[1]);
  printf("Filesize:           %d bytes\n", bmp->filesize);
  printf(
    "Reserved:           %02x %02x %02x %02x\n", bmp->reserved[0], bmp->reserved[1], bmp->reserved[2], bmp->reserved[3]
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
    printf("Colors:             [ ");
    for (uint32_t i = 0; i < bmp->n_colors; ++i) {
      printColor(bmp->colors[i]);
      if (i < bmp->n_colors - 1) printf(", ");
    }
    printf(" ]\n");
  }
  if (bmp->extra_data_size > 0) {
    printf("Extra data size:    %d\n", bmp->extra_data_size);
    printf("Extra data:         [ ");
    for (uint32_t i = 0; i < bmp->extra_data_size; ++i) {
      printf("%02x ", bmp->extra_data[i]);
    }
    printf("]\n");
  }
}

// Internal functions

void printColor(Color color) {
  printf("#%02x%02x%02x", color.r, color.g, color.b);
}

static bool freadWithPerror(FILE* file, void* dest, size_t size, const char* err) {
  if (fread(dest, size, 1, file) != 1) {
    perror(err);
    return false;
  }
  return true;
}

static bool parseBaseHeader(FILE* file, BMP bmp) {
  // I need to split the reading because in the strcut, the `filesize` field
  // will be shifted 2 bytes to align with the closest dword.
  if (!freadWithPerror(file, bmp, 2, "fread base")) return false;
  if (bmp->id[0] != 'B' || bmp->id[1] != 'M') return false;
  return freadWithPerror(file, &bmp->filesize, BASE_HEADER_SIZE - 2, "fread base");
}

static bool parseInfoHeader(FILE* file, BMP bmp) {
  if (!freadWithPerror(file, &bmp->info_header_size, sizeof(uint32_t), "fread info_size")) return false;
  return freadWithPerror(
    file, ((uint8_t*)&bmp->info_header_size) + sizeof(uint32_t), bmp->info_header_size - sizeof(uint32_t), "fread info"
  );
}

static bool parseColorTable(FILE* file, BMP bmp) {
  if (bmp->n_colors == 0) {
    bmp->colors = NULL;
    return true;
  }

  size_t color_bytes = bmp->n_colors * sizeof(Color);
  bmp->colors = malloc(color_bytes);
  if (!bmp->colors) {
    perror("malloc colors");
    return false;
  }

  return freadWithPerror(file, bmp->colors, color_bytes, "fread colors");
}

static bool parseExtraData(FILE* file, BMP bmp) {
  if (ftell(file) == bmp->offset) {
    bmp->extra_data_size = 0;
    bmp->extra_data = NULL;
    return true;
  }

  if (!freadWithPerror(file, &bmp->extra_data_size, sizeof(uint32_t), "fread extra size")) return false;

  if (bmp->extra_data_size == 0) {
    bmp->extra_data = NULL;
    return true;
  }

  bmp->extra_data = malloc(bmp->extra_data_size);
  if (!bmp->extra_data) {
    perror("malloc extra data");
    return false;
  }

  return freadWithPerror(file, bmp->extra_data, bmp->extra_data_size, "fread extra");
}

static bool parseImageData(FILE* file, BMP bmp) {
  if (fseek(file, bmp->offset, SEEK_SET) != 0) {
    perror("fseek");
    return false;
  }

  bmp->image = malloc(bmp->image_size);
  if (!bmp->image) {
    perror("malloc image");
    return false;
  }

  if (fread(bmp->image, 1, bmp->image_size, file) != bmp->image_size) {
    perror("fread image");
    return false;
  }

  return true;
}
