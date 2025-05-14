#include "SIS/sis.h"
#include "bmp/bmp.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
  const char* input_file = NULL;
  const char* output_file = "default-output-filename";

  bool distribute = false;
  bool recover = false;

  const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"header", no_argument, NULL, 'p'},
    {"output", required_argument, NULL, 'o'},
    {"input", required_argument, NULL, 'i'},
    {"distribute", no_argument, NULL, 'd'},
    {"recover", no_argument, NULL, 'r'},
    {0, 0, 0, 0}
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "ho:i:rdp", long_options, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: %s -i FILE <-r|-d> [...opts]\n", argv[0]);
      printf("  -i, --input   FILE    Required input file\n");
      printf("  -o, --output  FILE    Optional output file\n");
      printf("  -d, --distribute      Required (mutually exclusive with --recover)\n");
      printf("  -r, --recover         Required (mutually exclusive with --distribute)\n");
      printf("  -p, --header          Optional print input bmp header\n");
      printf("  -h, --help            Show this help message\n");
      return 0;
    case 'p':;
      BMP bmp = bmpParse(input_file);
      if (bmp == NULL) {
        fprintf(stderr, "Error parsing bmp");
        exit(EXIT_FAILURE);
      }
      bmpPrintHeader(bmp);
      bmpFree(bmp);
      return 0;
    case 'o':
      output_file = optarg;
      break;
    case 'i':
      input_file = optarg;
      break;
    case 'd':
      if (recover) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        exit(EXIT_FAILURE);
      }
      distribute = true;
      break;
    case 'r':
      if (distribute) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        exit(EXIT_FAILURE);
      }
      recover = true;
      break;
    default:
      fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (!input_file) {
    fprintf(stderr, "Error: input file is required.\n");
    fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (!distribute && !recover) {
    fprintf(stderr, "Error: one of --distribute or --recover must be specified.\n");
    exit(EXIT_FAILURE);
  }

  printf("Input file: %s\n", input_file);
  printf("Output file: %s\n\n", output_file);

  if (distribute) {
    BMP bmp = bmpParse(input_file);
    if (bmp == NULL) {
      fprintf(stderr, "Error parsing bmp");
      exit(EXIT_FAILURE);
    }
    bmpPrintHeader(bmp);
    BMP shadows[tot_shadows];
    sisShadows(bmp, min_shadows, tot_shadows, shadows);
    for (int i = 0; i < tot_shadows; ++i) {
      bmpWriteFile("asf", shadows[i]);
    }
    bmpFree(bmp);
  } else {
    BMP shadows[] = {
      bmpParse("../bmp-images/test-shadow-01.bmp"), bmpParse("../bmp-images/test-shadow-02.bmp"),
      bmpParse("../bmp-images/test-shadow-04.bmp"), bmpParse("../bmp-images/test-shadow-06.bmp"),
      bmpParse("../bmp-images/test-shadow-08.bmp"),
    };
    int32_t r = sizeof(shadows) / sizeof(shadows[0]);
    BMP secret = sisRecover(r, shadows);
    bmpPrintHeader(secret);
    bmpWriteFile(output_file, secret);
    for (int i = 0; i < r; ++i) bmpFree(shadows[i]);
    bmpFree(secret);
  }

  return 0;
}
