#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <png.h>
#include "typedef.h"

typedef struct libpng_interface
{
	png_structp png;
	png_infop info;
	png_infop endinfo;
}
libpng_interface;

typedef struct png_meta
{
	ui32 width;
	ui32 height;
	ui8 bit_depth;
}
png_meta;

typedef enum snes_depths
{
	snull,
	s2bpp,
	s4bpp,
	s8bpp,
	smode7
}
snes_depths;

#endif
