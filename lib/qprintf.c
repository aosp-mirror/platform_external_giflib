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
int GifQuietPrint = FALSE;
#else
int GifQuietPrint = TRUE;
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

    vsprintf(Line, Format, ArgPtr);
    va_end(ArgPtr);

    fputs(Line, stderr);
}
