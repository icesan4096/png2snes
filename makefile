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
