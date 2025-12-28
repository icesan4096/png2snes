#include <errno.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <png.h>

#include "include/typedef.h"
#include "include/common_types.h"
#include "include/macros.h"
#include "include/common_messages.h"

#include "convert.h"
#include "png_interface.h"



/* --------------------------------------------------------------------- */
/* Program switch struct                                                 */

typedef struct arg_switch
{
	char shorthand;
	char* long_name;
	char* description;
	char** arg_names;
	int (*action)(char**);
}
arg_switch;



/* --------------------------------------------------------------------- */
/* Program switch functions                                              */
int Set_Palette_Path(char** argv);
int Set_Graphics_Path(char** argv);
int Set_Bit_Depth(char** argv);
int Help_Screen_Switch(char** argv);
/* --------------------------------------------------------------------- */
/* Argument parsing and help screen functions                            */
void Print_Help_Screen();
void Print_Switch(arg_switch* switch_in);
int Process_Switch(arg_switch* switch_in, char*** arg_i, char** arg_max);
int Process_Argument(char* arg);
/* --------------------------------------------------------------------- */
/* Misc functions                                                        */
char* Get_Readable_Filename(char* path);



/* --------------------------------------------------------------------- */
/* Switches that the program can process                                 */

arg_switch main_switches[] =
{
	{ 'p', "palette", "Output the color palette at this path.", (char* []){ "path", NULL }, &Set_Palette_Path },
	{ 'g', "graphics", "Output the graphics at this path.", (char* []){ "path", NULL }, &Set_Graphics_Path },
	{ 'd', "depth", "Convert the graphics to this bit-depth.", (char* []){ "2, 4, 8, mode7", NULL }, &Set_Bit_Depth },
	{ '?', "help", "Display this help screen.", (char* []){ NULL }, &Help_Screen_Switch }
};
const size_t switches_array_size = sizeof(main_switches) / sizeof(arg_switch);



/* --------------------------------------------------------------------- */
/* Main entry point                                                      */

char* prog_name;	// Command passed to start the application

int main(int argc, char** argv)
{
	if (argc < 1) prog_name = "png2snes";
	else prog_name = *argv;

	if (argc < 2)
	{
	print_help:
		Print_Help_Screen();

		return 1;
	}

	char** argmax = argv + argc;	// End of the `argv` array.

	for (char** argi = argv + 1; argi < argmax; argi++)
	{
		char* arga = *argi;	// Current argument to process

		switch (*arga)
		{
		case '\0': goto print_help;
		case '-':
			/* Check for long-name argument */
			switch (arga[1])
			{
			case '\0': goto print_help;
			case '-':
				/* Long name argument, iterate through each switch's long name */
				{
					char* argb = arga + 2;	// Name to compare
					size_t switch_i;		// Switch index

					/* Iterate through each switch to find the matching name */
					for (switch_i = 0; switch_i < switches_array_size; switch_i++)
					{
						if (strcmp(argb, main_switches[switch_i].long_name)) continue;			// If strings don't match (if strcmp returns nonzero), skip switch.
						if (Process_Switch(&main_switches[switch_i], &argi, argmax)) return 1;	// Otherwise, proceed to process selected switch.

						break;
					}

					/* If no matches are found, print the help screen and exit */
					if (switch_i >= switches_array_size) goto print_help;
				}
				break;

			default:
				/* Shorthand argument, iterate through each switch's shorthand */
				{
					if (arga[2]) goto print_help;	// Each switch's shorthand should be only one character long (excluding -),
					//								// so if there's no NULL terminator after the character, then it's invalid.

					char cc = arga[1];		// Character to compare
					size_t switch_i;		// Switch index

					/* Iterate through each switch to find the matching character */
					for (switch_i = 0; switch_i < switches_array_size; switch_i++)
					{
						if (main_switches[switch_i].shorthand != cc) continue;					// If characters don't match, skip switch.
						if (Process_Switch(&main_switches[switch_i], &argi, argmax)) return 1;	// Otherwise, proceed to process selected switch

						break;
					}

					/* If no matches aree found, print the help screen and exit */
					if (switch_i >= switches_array_size) goto print_help;
				}
				break;
			}
			break;

		default:
			/* Non-switch argument */
			if (Process_Argument(arga)) return 1;
			break;
		}
	}

	return 0;
}



/* --------------------------------------------------------------------- */
/* Print the program's help screen.                                      */

void Print_Help_Screen()
{
	printf("Usage: %s", prog_name);

	for (size_t i = 0; i < switches_array_size; i++)
	{
		fputs(" [", stdout);
		Print_Switch(&main_switches[i]);
		fputc(']', stdout);
	}

	fputs(" <input path>\n\n", stdout);

	for (size_t i = 0; i < switches_array_size; i++)
	{
		arg_switch* switch_in = &main_switches[i];

		fputc('\t', stdout);
		Print_Switch(switch_in);

		printf("\t%s\n", switch_in->description);
	}
}



/* --------------------------------------------------------------------- */
/* Print a switch's name and description for use in the help screen      */

void Print_Switch(arg_switch* switch_in)
{
	if (switch_in->shorthand)
	{
		printf("-%c", switch_in->shorthand);
		if (switch_in->long_name) fputc(' ', stdout);
	}

	if (switch_in->long_name) printf("--%s", switch_in->long_name);

	char** aa = switch_in->arg_names;

	while (*aa) printf(" <%s>", *aa++);
}



/* --------------------------------------------------------------------- */
/* Collects arguments for the selected switch and executes its action    */

int Process_Switch(arg_switch* switch_in, char*** arg_i, char** arg_max)
{
	// jesus... i thought triple-pointers were useless.

	/* Count arguments in selected switch */
	if (!switch_in->action) { fprintf(stderr, "Functionality for switch %s is not present.\n", switch_in->long_name); return 1; }

	char** aa = switch_in->arg_names;
	char** bb = aa;

	while (*bb++);

	size_t cc = ((size_t)bb - (size_t)aa) / sizeof(char*);
	char* args_in[cc--];

	for (size_t i = 0; i < cc; i++)
	{
		*arg_i += 1;
		if (*arg_i >= arg_max) { fprintf(stderr, "Required parameters missing for argument %s.\n", switch_in->long_name); return 1; }

		args_in[i] = **arg_i;
	}
	args_in[cc] = NULL;

	return (*switch_in->action)(args_in);
}



/* --------------------------------------------------------------------- */
/* Main entry point for non-switch arugments                             */

char* palette_path = NULL;
char* graphics_path = NULL;
snes_depths target_depth = snull;

int Process_Argument(char* arg)
{
	/* Initial argument parsing */
	char* fname = Get_Readable_Filename(arg);
	if (!(graphics_path || palette_path)) { fprintf(stderr, "No outputs specified for input file %s.\n", fname); return 1; }

	/* Open requested file */
	FILE* filein = fopen(arg, "rb");
	if (!filein) { fprintf(stderr, errmsg_File_Open_Error, fname, strerror(errno)); return 1; }

	/* Connect file to libPNG -- we will not be interfacing with the file directly (except for when closing it). */	
	libpng_interface iface = Connect_File_To_libPNG(filein, fname);
	if (!iface.png) goto failure_close_file;

	/* "Save" program state for if libPNG fails internally.  A longjmp in libPNG will land here. */
	if (setjmp(png_jmpbuf(iface.png))) fail_puts_goto(failure_destroy_lpng_interface, "Internal libPNG error.\n");



	/* Give libPNG our I/O interface */
	png_init_io(iface.png, filein);
	png_set_sig_bytes(iface.png, PNG_HEADER_SAMPLE_SIZE);

	png_read_info(iface.png, iface.info);



	/* We're looking for PNG images formatted in a very specific way. */
	/* The images should be a multiple of 8 in each scale direction, */
	/* alongside using indexed color. */
	png_meta imeta = Verify_PNG(&iface, fname);
	if (!imeta.width) goto failure_destroy_lpng_interface;



	/* Palette export check */
	if (palette_path)
	{
		char* fname_out = Get_Readable_Filename(palette_path);
		FILE* fileout = fopen(palette_path, "wb");
		if (!fileout) fail_printf_goto(failure_destroy_lpng_interface, errmsg_File_Open_Error, fname_out, strerror(errno));

		int x = Convert_and_Save_Palette(&iface, fileout, fname_out);
		fclose(fileout);

		if (x) goto failure_destroy_lpng_interface;
		palette_path = NULL;
	}



	/* Graphics export check */
	if (graphics_path)
	{
		char* fname_out = Get_Readable_Filename(graphics_path);
		FILE* fileout = fopen(graphics_path, "wb");
		if (!fileout) fail_printf_goto(failure_destroy_lpng_interface, errmsg_File_Open_Error, fname_out, strerror(errno));

		int x = Convert_and_Save_Graphics(&iface, &imeta, target_depth, fileout, fname_out);
		fclose(fileout);

		if (x) goto failure_destroy_lpng_interface;
		graphics_path = NULL;
	}



	/* Success state */
	png_destroy_read_struct(&iface.png, &iface.info, &iface.endinfo);
	fclose(filein);
	return 0;



	/* Failure states */
failure_destroy_lpng_interface:
	png_destroy_read_struct(&iface.png, &iface.info, &iface.endinfo);

failure_close_file:
	fclose(filein);
	
	return 1;
}



/* --------------------------------------------------------------------- */

int Set_Palette_Path(char** argv)
{
	palette_path = *argv;
	return 0;
}



/* --------------------------------------------------------------------- */

int Set_Graphics_Path(char** argv)
{
	graphics_path = *argv;
	return 0;
}



/* --------------------------------------------------------------------- */

int Help_Screen_Switch(char** argv)
{
	Print_Help_Screen();
	return 1;
}



/* --------------------------------------------------------------------- */

int Set_Bit_Depth(char** argv)
{
	const char* err_Invalid_Arg = "Invalid argument processing switch -d.\n";

	char* arg = *argv;
	if (!(*arg)) { fputs(err_Invalid_Arg, stderr); return 1; }
	if (arg[1])
	{
		if (strcmp(arg, "mode7")) { fputs(err_Invalid_Arg, stderr); return 1; }

		target_depth = smode7;
		return 0;
	}

	switch (*arg)
	{
	case '2':
		target_depth = s2bpp;
		return 0;

	case '4':
		target_depth = s4bpp;
		return 0;

	case '8':
		target_depth = s8bpp;
		return 0;

	default:
		fputs(err_Invalid_Arg, stderr);
		return 1;
	}
}



/* --------------------------------------------------------------------- */
/* Get a friendly file name out of a (supposed) path to file             */

char* Get_Readable_Filename(char* path)
{
	register char* pp = path;
	register char* hit = NULL;

loop:
	switch (*pp++)
	{
	case '\0':
		if (hit) return hit; else return path;

	case '/':
	case '\\':
		hit = pp;

	default:
		goto loop;
	}
}
