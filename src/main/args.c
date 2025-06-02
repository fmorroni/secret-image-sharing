#define _GNU_SOURCE

#include "args.h"
#include "../bmp/bmp.h"
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void printHelp(const char* executable_name);
static uint8_t strToKRange(const char* str, const char* var_name);
static uint16_t strToUInt16(const char* str, const char* var_name);
static int countBmpFiles(const char* directory);
static void collectBmpFiles(Args* args, int needed_count);
static bool printHeader(const char* secret_filename);
static bool is_directory(const char* path);
__attribute__((noreturn)) static void clean_exit(Args* args, int err);
static Args* initArgs();
static void parseOptions(Args* args, int argc, char* argv[]);

Args* argsParse(int argc, char* argv[]) {
  Args* args = initArgs();
  parseOptions(args, argc, argv);

  if (!args->secret_filename) {
    fprintf(stderr, "Error: secret filename/path is required.\n");
    fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
    clean_exit(args, EXIT_FAILURE);
  }

  if (!args->distribute && !args->recover) {
    fprintf(stderr, "Error: one of --distribute or --recover must be specified.\n");
    clean_exit(args, EXIT_FAILURE);
  }

  if (args->min_shadows == 0) {
    fprintf(stderr, "Error: --min-shadows must be specified.\n");
    clean_exit(args, EXIT_FAILURE);
  }

  if (args->directory == NULL) {
    args->_directory_allocated = get_current_dir_name();
    if (args->_directory_allocated == NULL) {
      perror("getcwd");
      clean_exit(args, EXIT_FAILURE);
    }
    args->directory = args->_directory_allocated;
  }

  if (args->directory_out == NULL) args->directory_out = args->directory;
  else if (!is_directory(args->directory_out)) {
    fprintf(stderr, "Error: '%s' is not a valid directory.\n", args->directory_out);
    clean_exit(args, EXIT_FAILURE);
  }

  int bmps_in_dir = countBmpFiles(args->directory);
  if (args->tot_shadows == 0) args->tot_shadows = bmps_in_dir;
  else if (bmps_in_dir < args->tot_shadows) {
    fprintf(
      stderr, "Error: Not enough carrier images. Want %u shadows but found only %u carrier images.\n",
      args->tot_shadows, bmps_in_dir
    );
    clean_exit(args, EXIT_FAILURE);
  }

  if (args->tot_shadows < args->min_shadows) {
    fprintf(stderr, "Error: Not enough shadows (k: %u, n: %u) .\n", args->min_shadows, args->tot_shadows);
    clean_exit(args, EXIT_FAILURE);
  }

  uint8_t to_parse;
  if (args->distribute) to_parse = args->tot_shadows;
  else to_parse = args->min_shadows;

  args->dir_bmps = (BMP*)malloc(to_parse * sizeof(BMP));
  collectBmpFiles(args, to_parse);

  return args;
}

static Args* initArgs() {
  Args* args = malloc(sizeof(Args));
  if (!args) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  args->distribute = false;
  args->recover = false;
  args->secret_filename = NULL;
  args->min_shadows = 0;
  args->tot_shadows = 0;
  args->directory = NULL;
  args->directory_out = NULL;
  args->_directory_allocated = NULL;
  args->_parsed_bmps = 0;
  args->dir_bmps = NULL;
  args->seed = 0;
  return args;
}

static void parseOptions(Args* args, int argc, char* argv[]) {
  const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"print-header", no_argument, NULL, 'p'},
    {"distribute", no_argument, NULL, 'd'},
    {"recover", no_argument, NULL, 'r'},
    {"secret", required_argument, NULL, 's'},
    {"min-shadows", required_argument, NULL, 'k'},
    {"tot-shadows", required_argument, NULL, 'n'},
    {"dir", required_argument, NULL, 'D'},
    {"dir-out", required_argument, NULL, 'O'},
    {"seed", required_argument, NULL, 'S'},
    {0, 0, 0, 0}
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "hpdrs:k:n:D:O:S:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'h':
      printHelp(argv[0]);
      clean_exit(args, EXIT_SUCCESS);
    case 'p':
      if (!printHeader(args->secret_filename)) clean_exit(args, EXIT_FAILURE);
      else clean_exit(args, EXIT_SUCCESS);
    case 'd':
      if (args->recover) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        clean_exit(args, EXIT_FAILURE);
      }
      args->distribute = true;
      break;
    case 'r':
      if (args->distribute) {
        fprintf(stderr, "Error: --distribute and --recover are mutually exclusive.\n");
        clean_exit(args, EXIT_FAILURE);
      }
      args->recover = true;
      break;
    case 's':
      args->secret_filename = optarg;
      break;
    case 'k':
      errno = 0;
      args->min_shadows = strToKRange(optarg, "--min-shadows | -k");
      if (errno != 0) clean_exit(args, EXIT_FAILURE);
      break;
    case 'n':
      errno = 0;
      if (args->distribute) args->tot_shadows = strToKRange(optarg, "--tot-shadows | -n");
      if (errno != 0) clean_exit(args, EXIT_FAILURE);
      break;
    case 'D':
      args->directory = optarg;
      break;
    case 'O':
      args->directory_out = optarg;
      break;
    case 'S':
      errno = 0;
      args->seed = strToUInt16(optarg, "--seed | -S");
      if (errno != 0) clean_exit(args, EXIT_FAILURE);
      break;
    default:
      fprintf(stderr, "Try '%s --help' for usage.\n", argv[0]);
      clean_exit(args, EXIT_FAILURE);
    }
  }
}

void argsFree(Args* args) {
  // `free(NULL)` is a no-op so it's fine to have no check.
  free(args->_directory_allocated);
  for (int i = 0; i < args->_parsed_bmps; ++i) bmpFree(args->dir_bmps[i]);
  free((void*)args->dir_bmps);
  free(args);
}

static int countBmpFiles(const char* directory) {
  DIR* dir = opendir(directory);
  if (dir == NULL) {
    perror("opendir");
    return -1;
  }

  struct dirent* entry;
  int count = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) ++count;
  }

  closedir(dir);

  return count;
}

static void collectBmpFiles(Args* args, int needed_count) {
  DIR* dir = opendir(args->directory);
  if (dir == NULL) {
    perror("opendir");
    clean_exit(args, EXIT_FAILURE);
  }

  struct dirent* entry;

  int count = 0;
  while ((entry = readdir(dir)) != NULL && count < needed_count) {
    if (entry->d_type == DT_REG) {
      const char* name = entry->d_name;
      size_t full_len = strlen(args->directory) + 1 + strlen(name) + 1;
      char full_path[full_len];
      snprintf(full_path, full_len, "%s/%s", args->directory, name);
      printf("parsing bmp: `%s`...\n", full_path);
      args->dir_bmps[count] = bmpParse(full_path);
      if (args->dir_bmps[count] == NULL) {
        fprintf(stderr, "Error parsing bmp `%s`\n", full_path);
        clean_exit(args, EXIT_FAILURE);
      }
      args->_parsed_bmps = ++count;
    }
  }

  closedir(dir);
}

static void printHelp(const char* executable_name) {
  printf("Usage: %s <-r | -d> -s FILE -k NUM [options]\n", executable_name);
  printf("Options:\n");
  printf("  -h, --help               Show this help message and exit\n");
  printf("  -p, --print-header       Optional: Print the BMP header of the input image\n");
  printf("  -d, --distribute         Required: Distribute a secret into shadow images (mutually exclusive with -r)\n");
  printf("  -r, --recover            Required: Recover a secret from shadow images (mutually exclusive with -d)\n");
  printf("  -s, --secret FILE        Required:\n");
  printf("                             - With -d: input BMP image to hide/distribute\n");
  printf("                             - With -r: output file for the recovered secret\n");
  printf(
    "  -k, --min-shadows NUM    Required: Minimum number of shadow images needed to reconstruct the secret"
    " (2 ≤ NUM ≤ 255)\n"
  );
  printf("  -n, --tot-shadows NUM    Optional: Total number of shadow images to create (only if -d used)\n");
  printf("  -D, --dir DIR            Optional: Directory from which to read shadow images\n");
  printf("                             (default: current working directory)\n");
  printf("  -O, --dir-out DIR        Optional: Directory to write shadow images to (only if -d used)\n");
  printf("                             (default: the value provided to --dir)\n");
  printf("  -S, --seed NUM           Optional: Seed to use for permutation matrix.\n");
  printf("                             (default: 0 if -d used, `seed` from reserved bytes in shadow if -r used)\n");
}

static uint32_t strToNumInRange(const char* str, uint32_t min, uint32_t max, const char* var_name) {
  char* endptr;
  errno = 0;
  long val = strtol(str, &endptr, 10);

  if (errno != 0) {
    perror("strtoul");
    return 0;
  }
  if (*endptr != 0) {
    fprintf(stderr, "Invalid character '%c' in `%s`: %s\n", *endptr, var_name, str);
    errno = EINVAL;
    return 0;
  }
  if (val < min || val > max) {
    fprintf(stderr, "Value out of range [%u, %u] for `%s`: %li\n", min, max, var_name, val);
    errno = EINVAL;
    return 0;
  }

  return val;
}

static uint8_t strToKRange(const char* str, const char* var_name) {
  return (uint8_t)strToNumInRange(str, 2, UINT8_MAX, var_name);
}

static uint16_t strToUInt16(const char* str, const char* var_name) {
  return (uint16_t)strToNumInRange(str, 0, UINT16_MAX, var_name);
}

static bool printHeader(const char* secret_filename) {
  if (secret_filename == NULL) {
    fprintf(stderr, "Error: pass <-s FILE> before -p \n");
    return false;
  }
  BMP bmp = bmpParse(secret_filename);
  if (bmp == NULL) {
    fprintf(stderr, "Error parsing bmp\n");
    return false;
  }
  bmpPrintHeader(bmp);
  bmpFree(bmp);
  return true;
}

static bool is_directory(const char* path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0) {
    return false;
  }
  return S_ISDIR(statbuf.st_mode);
}

static void clean_exit(Args* args, int err) {
  argsFree(args);
  exit(err);
}
