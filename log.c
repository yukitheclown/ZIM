#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void Log_Formatted(int color, const char *file, int line, const char *format, ...){

	printf("\x1b[%im%s, Line %i\n", color, file, line);
	// printf("\x1b[%im", color);

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);

	printf("\x1b[0m\n");
	// printf("\x1b[0m");
}