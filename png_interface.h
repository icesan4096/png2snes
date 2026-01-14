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

#ifndef PNG_INTERFACE_H
#define PNG_INTERFACE_H

#include "include/common_types.h"

#define PNG_HEADER_SAMPLE_SIZE (8)

libpng_interface Connect_File_To_libPNG(FILE* filein, char* fname);
png_meta Verify_PNG(libpng_interface* iface, char* fname);

#endif
