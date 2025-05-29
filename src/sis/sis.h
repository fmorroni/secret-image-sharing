#ifndef SIS_H
#define SIS_H

#include "../bmp/bmp.h"
#include <stdint.h>

extern Color colors[256];

void sisShadows(BMP bmp, uint8_t min_shadows, uint8_t tot_shadows, BMP carrier_bmps[tot_shadows], uint16_t seed);
BMP sisRecover(uint8_t min_shadows, BMP shadows[min_shadows], uint16_t seed);

#endif
