#ifndef ARGS_H
#define ARGS_H

#include "../bmp/bmp.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Args {
  bool distribute;
  bool recover;
  const char* secret_filename;
  uint8_t min_shadows;
  uint8_t tot_shadows;
  const char* directory;
  const char* directory_out;
  char* _directory_allocated;
  uint8_t _parsed_bmps;
  BMP* dir_bmps;
  uint16_t seed;
} Args;

Args* argsParse(int argc, char* argv[]);
void argsFree(Args* args);

#endif
