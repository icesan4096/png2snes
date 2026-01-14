/* 
	png2snes, Command line graphics and color conversion utility using libpng
	Copyright (C) 2025  icesan4096

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
