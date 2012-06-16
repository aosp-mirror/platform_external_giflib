/*****************************************************************************

gifovly - composite images in a multi-image GIF as layers

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"gifovly"

static char
    *VersionStr =
	PROGRAM_NAME
	VERSION_COOKIE
	"	Eric Raymond,	"
	__DATE__ ",   " __TIME__ "\n"
	"(C) Copyright 1992 Eric Raymond.\n";
static char
    *CtrlStr =
	PROGRAM_NAME
	" t%-TransparentColor!d h%-";

int main(int argc, char **argv)
{
    int	k;
    GifFileType *GifFileIn, *GifFileOut = (GifFileType *)NULL;
    SavedImage *bp;
    int	TransparentColor = 0;
    bool Error, TransparentColorFlag = false, HelpFlag = false;

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
		&TransparentColorFlag, &TransparentColor,
		&HelpFlag)) != false) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (HelpFlag) {
	(void)fprintf(stderr, VersionStr, GIFLIB_MAJOR, GIFLIB_MINOR);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_SUCCESS);
    }

    if ((GifFileIn = DGifOpenFileHandle(0)) == NULL
	|| DGifSlurp(GifFileIn) == GIF_ERROR
	|| ((GifFileOut = EGifOpenFileHandle(1)) == (GifFileType *)NULL))
    {
	PrintGifError(GifFileOut->Error ? GifFileOut->Error : GifFileIn->Error);
	exit(EXIT_FAILURE);
    }

    GifFileOut->SWidth = GifFileIn->SWidth;
    GifFileOut->SHeight = GifFileIn->SHeight;
    GifFileOut->SColorResolution = GifFileIn->SColorResolution;
    GifFileOut->SBackGroundColor = GifFileIn->SBackGroundColor;
    GifFileOut->SColorMap = GifMakeMapObject(
				 GifFileIn->SColorMap->ColorCount,
				 GifFileIn->SColorMap->Colors);


    /* The output file will have exactly one image */
    GifMakeSavedImage(GifFileOut, &GifFileIn->SavedImages[0]);
    bp = &GifFileOut->SavedImages[0];
    for (k = 1; k < GifFileIn->ImageCount; k++)
    {
	register int	i, j;
	register unsigned char	*sp, *tp;

	SavedImage *ovp = &GifFileIn->SavedImages[k];

	for (i = 0; i < ovp->ImageDesc.Height; i++)
	{
	    tp = bp->RasterBits + (ovp->ImageDesc.Top + i) * bp->ImageDesc.Width + ovp->ImageDesc.Left;
	    sp = ovp->RasterBits + i * ovp->ImageDesc.Width;

	    for (j = 0; j < ovp->ImageDesc.Width; j++)
		if (!TransparentColorFlag || sp[j] != TransparentColor)
		    tp[j] = sp[j];
	}
    }

    if (EGifSpew(GifFileOut) == GIF_ERROR)
	PrintGifError(GifFileOut->Error);
    else if (DGifCloseFile(GifFileIn) == GIF_ERROR)
	PrintGifError(GifFileOut->Error);

    return 0;
}

/* end */
