#ifndef __CONSUILOG_H__
#define __CONSUILOG_H__

#include "config.h"
#include <cstdio>
#include <cstdarg>

#ifdef DEBUG
inline void LOG(const char* FN, const char* fmt, ...){
	FILE * f = fopen(FN,"at"); 
	va_list args;
	va_start(args, fmt);
	fprintf(f, fmt, args);
	va_end(args);
	fclose(f);
}
#else
inline void LOG(const char* , const char*, ...){}
#endif /* DEBUG */

#endif /* __CURSES_H__ */
