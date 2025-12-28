#include <png.h>

#include "include/common_types.h"
#include "include/macros.h"
#include "include/common_messages.h"

#include "png_interface.h"

#define MAXIMUM_DIMENSION (8192)

const char* err_Invalid_Header = "PNG header of file %s is invalid.\n";
const char* err_libPNG_Internal = "Internal libPNG error.\n";



/* --------------------------------------------------------------------- */
/* Connects an opened file to the libPNG library                         */

libpng_interface Connect_File_To_libPNG(FILE* filein, char* fname)
{
	/* Verify PNG */
	{
		/* Sample the header */
		png_byte header[PNG_HEADER_SAMPLE_SIZE];
		if (fread(header, sizeof(png_byte), PNG_HEADER_SAMPLE_SIZE, filein) != PNG_HEADER_SAMPLE_SIZE)
		{
			if (ferror(filein)) fprintf(stderr, errmsg_File_Read_Error, fname, ferror_str(filein));
			else fprintf(stderr, err_Invalid_Header, fname);

			goto failure_return_null;
		}

		/* Compare the signature using the libpng function */
		if (png_sig_cmp((png_const_bytep)header, 0, PNG_HEADER_SAMPLE_SIZE)) fail_printf_goto(failure_return_null, err_Invalid_Header, fname);
	}



	/* Prepare the components for libPNG to function -- this consists of a PNG struct and two info structs */
	png_structp pstruct_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pstruct_ptr) { fputs(err_libPNG_Internal, stderr); goto failure_return_null; }
	png_set_user_limits(pstruct_ptr, MAXIMUM_DIMENSION, MAXIMUM_DIMENSION);

	png_infop pinfo_ptr = png_create_info_struct(pstruct_ptr);
	if (!pinfo_ptr) fail_puts_goto(failure_destroy_png_struct, err_libPNG_Internal);

	png_infop pend_ptr = png_create_info_struct(pstruct_ptr);
	if (!pend_ptr) fail_puts_goto(failure_destroy_info_struct, err_libPNG_Internal);

	return (libpng_interface)
	{
		.png = pstruct_ptr,
		.info = pinfo_ptr,
		.endinfo = pend_ptr
	};

failure_destroy_info_struct:
	png_destroy_info_struct(NULL, &pinfo_ptr);

failure_destroy_png_struct:
	png_destroy_read_struct(&pstruct_ptr, NULL, NULL);

failure_return_null:
	return (libpng_interface)
	{
		.png = NULL,
		.info = NULL,
		.endinfo = NULL
	};
}



/* --------------------------------------------------------------------- */
/* Verify the opened PNG for low-level png2snes use                      */

png_meta Verify_PNG(libpng_interface* iface, char* fname)
{
	const ui32 actual_width = png_get_image_width(iface->png, iface->info);
	const ui32 actual_height = png_get_image_height(iface->png, iface->info);
	const ui8 bit_depth = png_get_bit_depth(iface->png, iface->info);
	
	if (png_get_color_type(iface->png, iface->info) != PNG_COLOR_TYPE_PALETTE)
		fail_printf_goto(failure_return, "Color type of image %s is not indexed.\n", fname);

	if (!actual_width) fail_printf_goto(failure_return, "Width of image %s is 0... how?\n", fname);
	if (!actual_height) fail_printf_goto(failure_return, "Height of image %s is 0... how?\n", fname);

	if (actual_width % 8) fail_printf_goto(failure_return, "Width of image %s is not a multiple of 8.\n", fname);
	if (actual_height % 8) fail_printf_goto(failure_return, "Height of image %s is not a multiple of 8.\n", fname);

	return (png_meta)
	{
		.width = actual_width,
		.height = actual_height,
		.bit_depth = bit_depth
	};



failure_return:
	return (png_meta)
	{
		.width = 0,
		.height = 0,
		.bit_depth = 0
	};
}
