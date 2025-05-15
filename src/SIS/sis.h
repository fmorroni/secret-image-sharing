#ifndef SIS_H
#define SIS_H

#include "../bmp/bmp.h"
#include <sys/types.h>

void sisShadows(BMP bmp, u_char min_shadows, u_char tot_shadows, BMP shadows[tot_shadows]);
BMP sisRecover(u_char min_shadows, BMP shadows[min_shadows]);

#endif
