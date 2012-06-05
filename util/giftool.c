/****************************************************************************

giftool.c - GIF transformation tool.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "getopt.h"
#include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"giftool"

static enum mode_t {
    copy,
    interlace,
    deinterlace,
    delay,
} mode;

int main(int argc, char **argv)
{
    extern char	*optarg;	/* set by getopt */
    extern int	optind;		/* set by getopt */
    int	i, status, delaytime = -1;
    GifFileType *GifFileIn, *GifFileOut = (GifFileType *)NULL;

    while ((status = getopt(argc, argv, "d:iI")) != EOF)
    {
	switch (status)
	{
	case 'd':
	    mode = delay;
	    delaytime = atoi(optarg);
	    break;

	case 'i':
	    mode = deinterlace;
	    break;

	case 'I':
	    mode = interlace;
	    break;

	default:
	    fprintf(stderr, "usage: giftool [-iI]\n");
	}
    }	

    if ((GifFileIn = DGifOpenFileHandle(0)) == NULL
	|| DGifSlurp(GifFileIn) == GIF_ERROR
	|| ((GifFileOut = EGifOpenFileHandle(1)) == (GifFileType *)NULL))
    {
	PrintGifError();
	exit(EXIT_FAILURE);
    }

    switch (mode) {
    case copy:
	/* default action: do nothing, copy straight through */
	break;

    case interlace:
	for (i = 0; i < GifFileIn->ImageCount; i++)
	    GifFileIn->SavedImages[i].ImageDesc.Interlace = true;
	break;

    case deinterlace:
	for (i = 0; i < GifFileIn->ImageCount; i++)
	    GifFileIn->SavedImages[i].ImageDesc.Interlace = false;
	break;

    case delay:
	for (i = 0; i < GifFileIn->ImageCount; i++)
	{
	    GraphicsControlBlock gcb;

	    DGifSavedExtensionToGCB(GifFileIn, i, &gcb);
	    gcb.DelayTime = delaytime;
	    EGifGCBToSavedExtension(&gcb, GifFileIn, i);
	}
	break;

    default:
	(void)fprintf(stderr, "giftool: unknown operation mode\n");
	exit(EXIT_FAILURE);
    }

    GifFileOut->SWidth = GifFileIn->SWidth;
    GifFileOut->SHeight = GifFileIn->SHeight;
    GifFileOut->SColorResolution = GifFileIn->SColorResolution;
    GifFileOut->SBackGroundColor = GifFileIn->SBackGroundColor;
    GifFileOut->SColorMap = MakeMapObject(
				 GifFileIn->SColorMap->ColorCount,
				 GifFileIn->SColorMap->Colors);

    for (i = 0; i < GifFileIn->ImageCount; i++)
	(void) MakeSavedImage(GifFileOut, &GifFileIn->SavedImages[i]);

    if (EGifSpew(GifFileOut) == GIF_ERROR)
	PrintGifError();
    else if (DGifCloseFile(GifFileIn) == GIF_ERROR)
	PrintGifError();

    return 0;
}

/* giftool.c ends here */
