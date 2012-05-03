/*****************************************************************************
 *   "Gif-Lib" - Yet another gif library.
 *
 * Written by:  Gershon Elber            IBM PC Ver 0.1,    Jun. 1989
 ******************************************************************************
 * Module to emulate a printf with a possible quiet (disable mode.)
 * A global variable GifQuietPrint controls the printing of this routine
 ******************************************************************************
 * History:
 * 12 May 91 - Version 1.0 by Gershon Elber.
 *****************************************************************************/


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "gif_lib.h"

#ifdef __MSDOS__
bool GifQuietPrint = false;
#else
bool GifQuietPrint = true;
#endif /* __MSDOS__ */

/*****************************************************************************
 * Same as fprintf to stderr but with optional print.
 *****************************************************************************/
void
GifQprintf(char *Format, ...) {
    char Line[128];
    va_list ArgPtr;

    va_start(ArgPtr, Format);

    if (GifQuietPrint)
        return;

    (void)vsnprintf(Line, sizeof(Line), Format, ArgPtr);
    va_end(ArgPtr);

    (void)fputs(Line, stderr);
}

#ifndef _GBA_NO_FILEIO
void
PrintGifError(void) {
    char *Err = GifErrorString();

    if (Err != NULL)
        fprintf(stderr, "\nGIF-LIB error: %s.\n", Err);
    else
        fprintf(stderr, "\nGIF-LIB undefined error %d.\n", GifError());
}
#endif /* _GBA_NO_FILEIO */
