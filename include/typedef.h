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

#ifndef TYPEDEF_H
#define TYPEDEF_H

typedef unsigned char ui8;
typedef unsigned short int ui16;
typedef unsigned int ui32;
typedef unsigned long long int ui64;

typedef signed char si8;
typedef signed short int si16;
typedef signed int si32;
typedef signed long long int si64;

typedef union
{
	ui16 whole;
	struct { ui8 low; ui8 high; } bytes;
}
amb16;

#endif
