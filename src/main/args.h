#ifndef ARGS_H
#define ARGS_H

#include "../bmp/bmp.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct Args {
  bool distribute;
  bool recover;
  const char* secret_filename;
  u_int8_t min_shadows;
  u_int8_t tot_shadows;
  const char* directory;
  char* _directory_allocated;
  u_int8_t _parsed_bmps;
  BMP* dir_bmps;
} Args;

Args* argsParse(int argc, char* argv[]);
void argsFree(Args* args);

#endif
