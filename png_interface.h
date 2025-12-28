#ifndef PNG_INTERFACE_H
#define PNG_INTERFACE_H

#include "include/common_types.h"

#define PNG_HEADER_SAMPLE_SIZE (8)

libpng_interface Connect_File_To_libPNG(FILE* filein, char* fname);
png_meta Verify_PNG(libpng_interface* iface, char* fname);

#endif
