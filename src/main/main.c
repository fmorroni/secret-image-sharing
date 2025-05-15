// #include "../SIS/sis.h"
#include "../bmp/bmp.h"
#include "args.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>

int main(int argc, char* argv[]) {
  Args* args = argsParse(argc, argv);
  // if (args.distribute) {
  //   BMP bmp = bmpParse(args.secret_filename);
  //   if (bmp == NULL) {
  //     fprintf(stderr, "Error parsing bmp `%s`", args.secret_filename);
  //     exit(EXIT_FAILURE);
  //   }
  //   bmpPrintHeader(bmp);
  //   BMP shadows[args.tot_shadows];
  //   sisShadows(bmp, args.min_shadows, args.tot_shadows, shadows);
  //   for (int i = 0; i < args.tot_shadows; ++i) {
  //     bmpWriteFile("asf", shadows[i]);
  //   }
  //   bmpFree(bmp);
  // } else {
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
  // }

  for (int i = 0; i < args->_parsed_bmps; ++i) {
    bmpPrintHeader(args->dir_bmps[i]);
  }

  argsFree(args);

  return 0;
}
