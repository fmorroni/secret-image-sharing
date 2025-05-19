#include "../SIS/sis.h"
#include "../bmp/bmp.h"
#include "args.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  Args* args = argsParse(argc, argv);
  if (args->distribute) {
    BMP bmp = bmpParse(args->secret_filename);
    if (bmp == NULL) {
      fprintf(stderr, "Error parsing bmp `%s`", args->secret_filename);
      exit(EXIT_FAILURE);
    }
    // bmpPrintHeader(bmp);
    sisShadows(bmp, args->min_shadows, args->tot_shadows, args->dir_bmps);
    for (int i = 0; i < args->tot_shadows; ++i) {
      char full_path[4096];
      snprintf(full_path, 4096, "%s/shadow-%03d.bmp", args->directory, i);
      printf("Saving `%s`...\n", full_path);
      bmpWriteFile(full_path, args->dir_bmps[i]);
    }
    bmpFree(bmp);
  } else {
    printf("min_shadows: %d, parsed_shadows: %d\n", args->min_shadows, args->_parsed_bmps);
    BMP secret = sisRecover(args->min_shadows, args->dir_bmps);
    bmpPrintHeader(secret);
    bmpWriteFile(args->secret_filename, secret);
    bmpFree(secret);
  }

  argsFree(args);

  return 0;
}
