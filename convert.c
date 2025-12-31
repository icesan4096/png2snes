#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <png.h>

#include "include/typedef.h"
#include "include/common_types.h"
#include "include/macros.h"
#include "include/common_messages.h"

#include "convert.h"

int Encode_Planar();
int Encode_Mode7();
int Flush_Planar();
int Flush_Mode7();


/* --------------------------------------------------------------------- */
/* Convert a 24-bit RGB color palette (PNG) to 15-bit BGR (SNES)         */

int Convert_and_Save_Palette(libpng_interface* iface, FILE* fout, char* fname)
{
	png_color* palette;		// will be set in png_get_PLTE
	si32 color_count;		// will be set in png_get_PLTE
	
	png_get_PLTE(iface->png, iface->info, &palette, &color_count);

	ui16 snes_palette[color_count];
	png_color* palette_end = palette + color_count;

	ui16* j = snes_palette;
	for (png_color* i = palette; i < palette_end; i++)
	{
		*j++ = 
			((i->blue  & 0b11111000) << 7) |
			((i->green & 0b11111000) << 2) |
			(i->red >> 3);
	}

	if (fwrite(snes_palette, sizeof(ui16), color_count, fout) != color_count)
	{
		fprintf(stderr, errmsg_File_Write_Error, fname, ferror_str(fout));
		return 1;
	}

	return 0;
}



/* --------------------------------------------------------------------- */
/* Convert the linear format of PNG to the planar (when not in mode7)    */
/* and tile-based format that the 5C77 and 5C78 use.                     */

amb16 planar_accumulator;	// Accumulator to store the two bitplanes for each line while reading the PNG
ui8 bitmap_accumulator;		// Accumulator to store the current PNG bitmap byte, containing 1~8 pixels' worth of data
ui8 linear_mask;			// Mask to filter out unneeded bits in the `bitmap_accumulator`
ui8 _1bpp_check;			// Mask to filter out the second bit if the input bitmap is 1bpp
FILE* fileout;				// File to write to in `Convert_and_Save_Graphics`
char* filename;				// File name to print after encountering an I/O error

int Convert_and_Save_Graphics(libpng_interface* iface, png_meta* imeta, snes_depths tdepth, FILE* fout, char* fname)
{
	fileout = fout;
	filename = fname;

	const size_t line_area = (imeta->width * imeta->bit_depth) >> 3;	// Size of a single PNG line
	const size_t memory_area = line_area * imeta->height;				// Size of the whole bitmap

	/* Allocate whole bitmap buffer */
	ui8* btbfr = calloc(memory_area, sizeof(ui8));						// Pointer to the bitmap buffer
	if (!btbfr) fail_puts_goto(failure_return, errmsg_Out_Of_Memory);

	/* ... and allocate an array of rows for libPNG. */
	ui8** lines_array = malloc(imeta->height * sizeof(ui8*));			// Array of lines for libpng to use -- this will be initialized by the loop below
	if (!lines_array) fail_puts_goto(failure_destroy_bitmap_buffer, errmsg_Out_Of_Memory);

	/* Configure the array so that libPNG can read the image into buffer */
	{
		ui8** lines_max = lines_array + imeta->height;	// End of the `lines_array` for the loop to end at
		ui8* current_line = btbfr;						// Pointer to the current line to write into the `lines_array`
		for (ui8** i = lines_array; i < lines_max;)
		{
			*i++ = current_line;
			current_line += line_area;
		}
	}

	/* Read the raw bitmap data into memory. */
	png_read_image(iface->png, lines_array);



	/* PNG buffer navigation */
	{
		register ui8* head = btbfr;							// Pointer to the next pixel(s) to evaluate

		const ui8 shift_amount = imeta->bit_depth;			// Amount to shift to find the next pixel
		const size_t line_skip = line_area - shift_amount;	// Amount of memory to skip at the end of an 8px line

		const size_t eight_lines = line_area << 3;			// Amount of memory to rewind to repeat the current tile for the next composite plane

		const size_t next_row = eight_lines - line_area;	// Amount of memory to skip to get from the 2nd PNG line to the next row of tiles

		if (imeta->bit_depth == 1) _1bpp_check = 0; else _1bpp_check = 0b00000010;

		ui8 total_planes;							// Total number of bits per pixel to write
		int (*encode_method)() = &Encode_Planar;	// Function to collect pixel data (planar) or convert it to a linear pixel (mode7)
		int (*flush_method)() = &Flush_Planar;		// Function to write collected pixel data to disk (planar)

		switch (tdepth)
		{
		case smode7:
			encode_method = &Encode_Mode7;
			flush_method = &Flush_Mode7;

			switch (shift_amount)
			{
			case 1:
				linear_mask = 0b00000001;
				break;
			case 2:
				linear_mask = 0b00000011;
				break;
			case 4:
				linear_mask = 0b00001111;
				break;
			default:
				linear_mask = 0b11111111;
				break;
			}

		case s2bpp:
			total_planes = 2;
			break;

		case s4bpp:
			total_planes = 4;
			break;

		case s8bpp:
			total_planes = 8;
			break;

		default:
			fail_puts_goto(failure_destroy_lines_array, "Target bit-depth is not supported.\n");
		}

		ui8 shift_limit;	// Limit to avoid writing excess bitplanes
		if (shift_amount > total_planes) shift_limit = total_planes; else shift_limit = shift_amount;

		/* Conversion loop */

		/* Write rows of SNES format tiles to disk. */
		for (ui8* image_end = btbfr + memory_area; head < image_end;)	// for (ui32 row_i = 0; row_i < tiles_down; row_i++)
		{
			/* Write a line of SNES format tiles to disk. */
			for (ui8* row_end = head + line_area; head < row_end;)	// for (ui32 tile_i = 0; tile_i < tiles_across; tile_i++)
			{
				/* Write a single SNES format tile to disk. */
				ui8 plane_i = 0;
				do
				{
					/* Write a single composite plane of a SNES format tile to disk. */
					for (ui8* tile_bottom = head + eight_lines; head < tile_bottom;)	// for (ui8 line_i = 0; line_i < 8; line_i++)
					{
						/* Write a single line of a SNES format tile to disk. */
						ui8 shifted_bits = 8;
						planar_accumulator = (amb16){ .whole = 0 };

						for (ui8 pixel_i = 0; pixel_i < 8; pixel_i++)
						{
							if (shifted_bits >= 8)
							{
								bitmap_accumulator = (*head++) >> plane_i;	// head: move along length of the tile
								shifted_bits = 0;
							}

							if ((*encode_method)()) goto failure_destroy_lines_array;

							bitmap_accumulator >>= shift_amount;
							shifted_bits += shift_amount;
						}

						if ((*flush_method)()) goto failure_destroy_lines_array;

						head += line_skip;			// head: skip rest of PNG line and move down to next line of same tile
					}

					head -= eight_lines;	// head: go back up to top-left corner of tile to process the next compositing pass
					plane_i += 2;
				}
				while (plane_i < shift_limit);

				/* Filler planes for when supplied PNG is at a lower bit depth */
				while (plane_i < total_planes)
				{
					for (ui8 i = 0; i < 16; i++)
						if (fputc(0, fileout) == EOF) goto failure_ioerror_common;

					plane_i += 2;
				}

				head += shift_amount;	// head: move 8 pixels across to next tile
			}

			head += next_row;	// head: move 7 PNG lines down to land on top-left corner of the first tile of the second row
		}
	}

	if (fflush(fileout)) goto failure_ioerror_common;

	

	/* Success state */

	free(btbfr);
	free(lines_array);

	return 0;



	/* Failure states */
failure_ioerror_common:
	fprintf(stderr, errmsg_File_Write_Error, fname, ferror_str(fileout));

failure_destroy_lines_array:
	free(lines_array);

failure_destroy_bitmap_buffer:
	free(btbfr);

failure_return:
	return 1;
}

/* --------------------------------------------------------------------- */
int Encode_Planar()
{
	planar_accumulator.whole <<= 1;
	planar_accumulator.bytes.low  |=  bitmap_accumulator & 0b00000001;
	planar_accumulator.bytes.high |= (bitmap_accumulator & _1bpp_check) >> 1;

	return 0;
}

/* --------------------------------------------------------------------- */
int Encode_Mode7()
{
	if (fputc(bitmap_accumulator & linear_mask, fileout) == EOF)
	{
		fprintf(stderr, errmsg_File_Write_Error, filename, ferror_str(fileout));
		return 1;
	}

	return 0;
}

/* --------------------------------------------------------------------- */
int Flush_Planar()
{
	if (fputc(planar_accumulator.bytes.low,  fileout) == EOF) goto failure_ioerror_common;
	if (fputc(planar_accumulator.bytes.high, fileout) == EOF) goto failure_ioerror_common;

	return 0;

failure_ioerror_common:
	fprintf(stderr, errmsg_File_Write_Error, filename, ferror_str(fileout));

	return 1;
}

/* --------------------------------------------------------------------- */
int Flush_Mode7()
{
	return 0;
}
