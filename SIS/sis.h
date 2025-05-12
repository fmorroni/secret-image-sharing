#ifndef SIS_H
#define SIS_H

#include "../bmp/bmp.h"
#include <sys/types.h>

void sisShadows(BMP bmp, u_char r, u_char n);
BMP sisRecover(u_char r, BMP shadows[r]);

#endif
