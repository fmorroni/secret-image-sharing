#ifndef SIS_H
#define SIS_H

#include "../bmp/bmp.h"
#include <stdint.h>

void sisShadows(BMP bmp, uint8_t min_shadows, uint8_t tot_shadows, BMP shadows[tot_shadows]);
BMP sisRecover(uint8_t min_shadows, BMP shadows[min_shadows]);

#endif
