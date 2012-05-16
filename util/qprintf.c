/*****************************************************************************
 *   "Gif-Lib" - Yet another gif library.
 *
 * Written by:  Gershon Elber            IBM PC Ver 0.1,    Jun. 1989
 ******************************************************************************
 * Module to emulate a printf with a possible quiet (disable mode.)
 * A global variable GifNoisyPrint controls the printing of this routine
 ******************************************************************************
 * History:
 * 12 May 91 - Version 1.0 by Gershon Elber.
 *****************************************************************************/


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "gif_lib.h"

bool GifNoisyPrint = false;

/*****************************************************************************
 * Same as fprintf to stderr but with optional print.
 *****************************************************************************/
void
GifQprintf(char *Format, ...) {
    char Line[128];
    va_list ArgPtr;

    va_start(ArgPtr, Format);

    if (GifNoisyPrint) {
	(void)vsnprintf(Line, sizeof(Line), Format, ArgPtr);
	(void)fputs(Line, stderr);
    }

    va_end(ArgPtr);
}

void
PrintGifError(void) {
    char *Err = GifErrorString();

    if (Err != NULL)
        fprintf(stderr, "\nGIF-LIB error: %s.\n", Err);
    else
        fprintf(stderr, "\nGIF-LIB undefined error %d.\n", GifError());
}
