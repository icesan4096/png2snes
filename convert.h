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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include "include/common_types.h"

int Convert_and_Save_Palette(libpng_interface* iface, FILE* fout, char* fname);
int Convert_and_Save_Graphics(libpng_interface* iface, png_meta* imeta, snes_depths tdepth, FILE* fout, char* fname);

#endif