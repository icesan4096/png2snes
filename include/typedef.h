// PNG2SNES typedef file

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
