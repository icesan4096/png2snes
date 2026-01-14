#	png2snes, Command line graphics and color conversion utility using libpng
#	Copyright (C) 2025  icesan4096
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.

COMMON_INCLUDES=include/typedef.h include/common_types.h include/macros.h include/common_messages.h

CC=gcc
CCARGS=-Wall -g
CCINVOKE=$(CC) $(CCARGS) -c -o $@ $^
LDARGS=-lpng

.PHONY: all clean

all: build/png2snes

clean:
	-rm *.o



%.o: %.c | %.h $(COMMON_INCLUDES)
	$(CCINVOKE)

main.o: main.c | convert.h png_interface.h $(COMMON_INCLUDES)
	$(CCINVOKE)

build/png2snes: main.o convert.o png_interface.o
	@if test -d build; then true; else mkdir build; fi
	$(CC) -o $@ $^ $(LDARGS)
