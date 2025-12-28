#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <string.h>

#define ferror_str(f) strerror(ferror(f))
#define fail_puts_goto(label, string) { fputs(string, stderr); goto label; }
#define fail_printf_goto(label, ...) { fprintf(stderr, __VA_ARGS__); goto label; }
#define Message_Declare(mname, string) inline char* mname() { return string; }

#endif
