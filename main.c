#include "SIS/sis.h"
#include "bmp/bmp.h"
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
  const char* input_file = NULL;
  const char* output_file = "default-output-filename";

  const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"output", required_argument, NULL, 'o'},
    {"input", required_argument, NULL, 'i'},
    {0, 0, 0, 0}
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "ho:i:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: %s -i FILE [-o FILE]\n", argv[0]);
      printf("  -i, --input   FILE    Required input file\n");
      printf("  -o, --output  FILE    Optional output file\n");
      printf("  -h, --help            Show this help message\n");
      return 0;
    case 'o':
      output_file = optarg;
      break;
    case 'i':
      input_file = optarg;
      break;
    default:
      fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
      return 1;
    }
  }

  if (!input_file) {
    fprintf(stderr, "Error: input file is required.\n");
    fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
    return 1;
  }

  printf("Input file: %s\n", input_file);
  printf("Output file: %s\n\n", output_file);

  BMP bmp = bmpParse(input_file);
  if (bmp == NULL) {
    fprintf(stderr, "Error parsing header");
    exit(EXIT_FAILURE);
  }
  bmpPrintHeader(bmp);

  // unsigned char* img = bmpImage(bmp);
  // int width = bmpWidth(bmp);
  // int height = bmpHeight(bmp);
  // for (int row = 0; row < height / 2; ++row) {
  //   for (int col = 0; col < width / 2; ++col) {
  //     img[row * width + col] ^= 0xFF;
  //   }
  // }
  // for (int row = height / 2; row < height; ++row) {
  //   for (int col = width / 2; col < width; ++col) {
  //     img[row * width + col] ^= 0xFF;
  //   }
  // }

  // int32_t img_size = bmpImageSize(bmp);
  // char factor = 0x1E;
  // for (int i = 0; i < img_size; i += 1) {
  //   if ((int)img[i] + factor <= 255) img[i] += factor;
  //   else img[i] = 255;
  // }

  sisShadows(bmp, 5, 10);

  bmpFree(bmp);

  return 0;
}
