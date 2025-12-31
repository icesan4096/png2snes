# png2snes
Command line graphics and color conversion utility using libpng

This tool converts the linear-format graphics and colors of paletted PNG images to a planar, tile-based format used by the 5C77 and 5C78 "S-PPU" chips, primarily found in the Super Nintendo Entertainment System (SNES) and Super Family Computer (Super Famicom / SFC) systems.

# Usage
png2snes can only be run from the command line.  The syntax for command line use is as follows:
```
./png2snes   <[-p OR --palette <output path>] [-g OR --graphics <output path>] [-d OR --depth <2, 4, 8, OR mode7>]>   <input path>
```
- `-p OR --palette <output path>` - Convert the PNG's color palette and output the results to `<output path>`.
- `-g OR --graphics <output path>` - Convert the PNG's graphics and output the results to `<output path>`.
- `-d OR --depth <2, 4, 8, mode7>` - Output the converted PNG's graphics at the following bit-depth and format.

The main purpose of this tool is to be used in build scripts and makefiles.  For example, a make recipe to convert the PNG and generate source code for the data, assuming the program `png2snes` is in PATH, is as follows:
```makefile
# Convert indexed PNG into 2bpp planar graphics
%.2bpp.grp.bin: %.png
	png2snes -g $@ -d 2 $^

# Extract indexed PNG's palette and convert to 15-bit BGR format
%.2bpp.pal.bin: %.png
	png2snes -p $@ $^
```

# Requirements
Pre-built binaries require the libpng library to run, and building from source additionally requires the development files.  How to obtain libpng depends on the operating system in use.  For example, on Debian, `apt` can be used to search for and install these requirements.  If all else fails, you may download pre-built binaries or source code for libpng on the [website](https://www.libpng.org/pub/png/libpng.html).

Additionally, GNU coreutils, make, bash, and GCC are required to build from source.  On GNU Linux based systems, coreutils is installed by default, there's a good chance make and bash are installed as well, and GCC can be installed through binutils.  On Windows systems, these need to be installed through a subsystem, such as w64devkit, Cygwin, or WSL.  MinGW also needs to be installed onto these subsystems to build executables native to Windows.

When all the requirements are met, simply run `make` on the root of the repository.

You may try to build with GCC alternatives like clang, but functionality and results haven't been tested with such alternatives (I'm personally concerned about whether or not the `amb16` data type (defined in `include/typedef.h`) will work.).

# Technical Explanation
## PNG Images
The raw bitmap data for PNG images (after decompression and deinterlacing) is relatively straightforward to understand.  The bitmap data is one giant buffer the size of the image's area multiplied by the number of channels and a fraction of the image's bits per pixel.  For paletted images, the image data contains only one channel of a set number of bits per pixel (defined in the PNG's info chunk).  The buffer contains indexes into the image's color table from left to right, and top to bottom.

```
  w = h = 16; a = wh = 256
0          7          15
  +--------+--------+
  |        |        |
  |        |        |
  |        |        |
  |        |        |
  +--------+--------+ 127
  |        |        |
  |        |        |
  |        |        |
  |        |        |
  +--------+--------+
                      255
```

## SNES Format Graphics
### Mode 7
SNES format graphics can be seen as having one buffer for each 8x8 cell of the image, called a tile.  The most basic tile to explain is the Mode 7 format tile.  These essentially function like individual 8x8 PNG bitmap buffers at 8 bits per pixel.  Each byte is either an index in the S-PPU's Color Graphics RAM (CGRAM) or a 2-3-3bpp BGR encoded color depending on whether Direct Color mode is turned on or not.

```
0            7    64          71
  +--------+        +--------+
  |        |        |        |
  |        |        |        |
  |        |        |        |
  |        |        |        |
  +--------+        +--------+
            63               127

128        135    192        199
  +--------+        +--------+
  |        |        |        |
  |        |        |        |
  |        |        |        |
  |        |        |        |
  +--------+        +--------+
           191               255
```

### Planar Tiles
A more complex format is the standard 2 bit-per-pixel tile found in Modes 0, 1, 4, and 5.  Each horizontal row of each tile is its own set of two 8-bit planes.  The first plane contains the lower bit of each pixel, while the second plane contains the higher bit.  Eight sets of bit-planes make eight rows of eight two-bit pixels, which makes a full tile.

```
    +--------+     +--------+     +-----------------------+
 0. |01234567|  1. |89ABCDEF|  =  |80 91 A2 B3 C4 D5 E6 F7|
    +--------+     +--------+     +-----------------------+
 2. |GHIJKLMN|  3. |OPQRSTUV|  =  |OG PH QI RJ SK TL UM VN|
    +--------+     +--------+     +-----------------------+
                          .  .  .
    +--------+     +--------+     +-----------------------+
14. |WXYZabcd| 15. |efghijkl|  =  |eW fX gY hZ ia jb kc ld|
    +--------+     +--------+     +-----------------------+
```

### Compositing to Higher Bit Depths
4 bit-per-pixel and 8 bit-per-pixel tiles build upon this formula.  4 bpp tiles are the composite result of two 2 bpp tiles next to one another in memory, and 8 bpp tiles are the composite result of two 4 bpp tiles next to one another, and by extension four 2 bpp tiles next to one another.

```
    +--------+     +--------+     +--------+     +--------+     +---------------------------------------+
 0. |01234567|  1. |89ABCDEF| 16. |GHIJKLMN| 17. |OPQRSTUV|  =  |OG80 PH91 QIA2 RJB3 SKC4 TLD5 UME6 VNF7|
    +--------+     +--------+     +--------+     +--------+     +---------------------------------------+
                                            .  .  .
    +--------+     +--------+     +--------+     +--------+     +---------------------------------------+
14. |WXYZabcd| 15. |efghijkl| 30. |mnopqrst| 31. |uvwxyz~:|  =  |umeW vnfX wogY xphZ yqia zrjb ~skc :tld|
    +--------+     +--------+     +--------+     +--------+     +---------------------------------------+
```

## SNES Format Color
The 15 bit-per-color BGR format allows up to 32,768 unique combinations of blue, green, and red component values.  The bits are arranged as follows:
```
  0BBB BBGG GGGR RRRR
   ||| |||| |||+-++++- Red component
   ||| ||++-+++------- Green component
   +++-++------------- Blue component
```
