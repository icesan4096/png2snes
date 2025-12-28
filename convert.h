#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include "include/common_types.h"

int Convert_and_Save_Palette(libpng_interface* iface, FILE* fout, char* fname);
int Convert_and_Save_Graphics(libpng_interface* iface, png_meta* imeta, snes_depths tdepth, FILE* fout, char* fname);

#endif