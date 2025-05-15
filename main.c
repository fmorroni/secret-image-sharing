#define _GNU_SOURCE

#include "SIS/sis.h"
#include "bmp/bmp.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Args {
  bool distribute;
  bool recover;
  const char* secret_filename;
  int min_shadows;
  int tot_shadows;
  const char* directory;
  char* directory_allocated;
} Args;

Args argsParse(int argc, char* argv[]);
void argsFree(Args args);
void printHelp(const char* executable_name);
u_char strToUChar(const char* str);

int main(int argc, char* argv[]) {
  Args args = argsParse(argc, argv);
  if (args.distribute) {
    BMP bmp = bmpParse(args.secret_filename);
    if (bmp == NULL) {
      fprintf(stderr, "Error parsing bmp `%s`", args.secret_filename);
      exit(EXIT_FAILURE);
    }
    bmpPrintHeader(bmp);
    BMP shadows[args.tot_shadows];
    sisShadows(bmp, args.min_shadows, args.tot_shadows, shadows);
    for (int i = 0; i < args.tot_shadows; ++i) {
      bmpWriteFile("asf", shadows[i]);
    }
    bmpFree(bmp);
  } else {
    // BMP shadows[] = {
    //   bmpParse("../bmp-images/test-shadow-01.bmp"), bmpParse("../bmp-images/test-shadow-02.bmp"),
    //   bmpParse("../bmp-images/test-shadow-04.bmp"), bmpParse("../bmp-images/test-shadow-06.bmp"),
    //   bmpParse("../bmp-images/test-shadow-08.bmp"),
    // };
    // int32_t r = sizeof(shadows) / sizeof(shadows[0]);
    // BMP secret = sisRecover(r, shadows);
    // bmpPrintHeader(secret);
    // bmpWriteFile(secret_filename, secret);
    // for (int i = 0; i < r; ++i) bmpFree(shadows[i]);
    // bmpFree(secret);
  }

  argsFree(args);

  return 0;
}

Args argsParse(int argc, char* argv[]) {
  const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"header", no_argument, NULL, 'p'},
    {"distribute", no_argument, NULL, 'd'},
    {"recover", no_argument, NULL, 'r'},
    {"secret", required_argument, NULL, 's'},
    {"min-shadows", required_argument, NULL, 'k'},
    {"tot-shadows", required_argument, NULL, 'n'},
    {"dir", required_argument, NULL, 'D'},
    {0, 0, 0, 0}
  };

  Args args = {
    .distribute = false,
    .recover = false,
    .secret_filename = NULL,
    .min_shadows = -1,
    .tot_shadows = -1,
    .directory = NULL,
    .directory_allocated = NULL
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "hpdrs:k:n:D:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printHelp(argv[0]);
      exit(EXIT_SUCCESS);
    case 'p':;
      BMP bmp = bmpParse(args.secret_filename);
      if (bmp == NULL) {
        fprintf(stderr, "Error parsing bmp");
        exit(EXIT_FAILURE);
      }
      bmpPrintHeader(bmp);
      bmpFree(bmp);
      exit(EXIT_SUCCESS);
    case 'd':
      if (args.recover) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        exit(EXIT_FAILURE);
      }
      args.distribute = true;
      break;
    case 'r':
      if (args.distribute) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        exit(EXIT_FAILURE);
      }
      args.recover = true;
      break;
    case 's':
      args.secret_filename = optarg;
      break;
    case 'k':
      args.min_shadows = strToUChar(optarg);
      break;
    case 'n':
      args.tot_shadows = strToUChar(optarg);
      break;
    case 'D':
      args.directory = optarg;
      break;
    default:
      fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (!args.secret_filename) {
    fprintf(stderr, "Error: secret filename/path is required.\n");
    fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (!args.distribute && !args.recover) {
    fprintf(stderr, "Error: one of --distribute or --recover must be specified.\n");
    exit(EXIT_FAILURE);
  }

  if (args.min_shadows < 2) {
    fprintf(stderr, "Error: --min-shadows must be specified and be greater than 2.\n");
    exit(EXIT_FAILURE);
  }

  if (args.directory == NULL) {
    args.directory_allocated = get_current_dir_name();
    if (args.directory_allocated == NULL) {
      perror("getcwd");
      exit(EXIT_FAILURE);
    }
  }

  if (args.tot_shadows < 0) {
    // TODO: handle properly.
    args.tot_shadows = args.min_shadows;
  }

  return args;
}

void argsFree(Args args) {
  // `free(NULL)` is a no-op so it's fine to have no check.
  free(args.directory_allocated);
}

void printHelp(const char* executable_name) {
  printf("Usage: %s <-r | -d> -s FILE -k NUM [options]\n", executable_name);
  printf("Options:\n");
  printf("  -h, --help               Show this help message and exit\n");
  printf("  -p, --header             Optional: Print the BMP header of the input image\n");
  printf("  -d, --distribute         Distribute a secret into shadow images (mutually exclusive with -r)\n");
  printf("  -r, --recover            Recover a secret from shadow images (mutually exclusive with -d)\n");
  printf("  -s, --secret FILE        Required:\n");
  printf("                             - With -d: input BMP image to hide/distribute\n");
  printf("                             - With -r: output file for the recovered secret\n");
  printf(
    "  -k, --min-shadows NUM    Required: Minimum number of shadow images needed to reconstruct the secret (2 ≤ "
    "NUM ≤ 255)\n"
  );
  printf("  -n, --tot-shadows NUM    Optional: Total number of shadow images to create\n");
  printf("  -D, --dir DIR            Optional: Directory to read/write shadow images\n");
  printf("                             (default: current working directory)\n");
}

u_char strToUChar(const char* str) {
  char* endptr;
  errno = 0;
  long val = strtol(str, &endptr, 10);

  if (errno != 0) {
    perror("strtoul");
    exit(errno);
  }
  if (*endptr != '\0') {
    fprintf(stderr, "Invalid character '%c' in input: %s\n", *endptr, str);
    exit(EXIT_FAILURE);
  }
  if (val < 0 || val > 255) {
    fprintf(stderr, "Value out of range for unsigned char: %lu\n", val);
    exit(EXIT_FAILURE);
  }

  return (u_char)val;
}
