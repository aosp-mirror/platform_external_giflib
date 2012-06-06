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

#define MAX_OPERATIONS	256
#define MAX_IMAGES	2048

struct operation {
    enum {
	delaytime,
	background,
	interlace,
	deinterlace,
	transparent,
	userinput_on,
	userinput_off,
	disposal,
    } mode;    
    union {
	int delay;
	int color;
	int dispose;
    };
};

int main(int argc, char **argv)
{
    extern char	*optarg;	/* set by getopt */
    extern int	optind;		/* set by getopt */
    struct operation operations[MAX_OPERATIONS];
    struct operation *top = operations;
    int selected[MAX_IMAGES], nselected = 0;
    bool have_selection = false;
    char *cp;
    int	i, status;
    GifFileType *GifFileIn, *GifFileOut = (GifFileType *)NULL;
    struct operation *op;

    /*
     * Gather operations from the command line.  We use regular
     * getopt(3) here rather than Gershom's argument getter because
     * preserving the order of operations is important.
     */
    while ((status = getopt(argc, argv, "bd:iIn:tuUx:")) != EOF)
    {
	if (top >= operations + MAX_OPERATIONS) {
	    (void)fprintf(stderr, "giftool: too many operations.");
	    exit(EXIT_FAILURE);
	}

	switch (status)
	{
	case 'b':
	    top->mode = background;
	    top->color = atoi(optarg);

	case 'd':
	    top->mode = delaytime;
	    top->delay = atoi(optarg);
	    break;

	case 'i':
	    top->mode = deinterlace;
	    break;

	case 'I':
	    top->mode = interlace;
	    break;

	case 'n':
	    have_selection = true;
	    nselected = 0;
	    cp = optarg;
	    for (;;)
	    {
		size_t span = strspn(cp, "0123456789");

		if (span > 0)
		{
		    selected[nselected++] = atoi(cp)-1;
		    cp += span;
		    if (*cp == '\0')
			break;
		    else if (*cp == ',')
			continue;
		}

		(void) fprintf(stderr, "giftool: bad selection.\n");
		exit(EXIT_FAILURE);
	    }
	    break;

	case 'u':
	    top->mode = userinput_off;
	    break;

	case 'U':
	    top->mode = userinput_on;
	    break;

	case 'x':
	    top->mode = disposal;
	    top->dispose = atoi(optarg);
	    break;

	default:
	    fprintf(stderr, "usage: giftool [-b color] [-d delay] [-iI] [-t color] -[uU] [-x disposal]\n");
	    break;
	}

	++top;
    }	

    /* read in a GIF */
    if ((GifFileIn = DGifOpenFileHandle(0)) == NULL
	|| DGifSlurp(GifFileIn) == GIF_ERROR
	|| ((GifFileOut = EGifOpenFileHandle(1)) == (GifFileType *)NULL))
    {
	PrintGifError();
	exit(EXIT_FAILURE);
    }

    /* if the selection is defaulted, compute it; otherwise bounds-check it */
    for (i = nselected = 0; i < GifFileIn->ImageCount; i++)
	if (!have_selection)
	    selected[nselected++] = i;
	else if (selected[nselected++] >= GifFileIn->ImageCount)
	{
	    (void) fprintf(stderr, "giftool: selection index out of bounds.\n");
	    exit(EXIT_FAILURE);
	}

    /* perform the operations we've gathered */
    for (op = operations; op < top; op++)
	switch (op->mode)
	{
	case background:
	    GifFileIn->SBackGroundColor = op->color; 
	    break;

	case delaytime:
	    for (i = 0; i < nselected; i++)
	    {
		GraphicsControlBlock gcb;

		DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
		gcb.DelayTime = op->delay;
		EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
	    }
	    break;

	case interlace:
	    for (i = 0; i < nselected; i++)
		GifFileIn->SavedImages[selected[i]].ImageDesc.Interlace = true;
	    break;

	case deinterlace:
	    for (i = 0; i < nselected; i++)
		GifFileIn->SavedImages[selected[i]].ImageDesc.Interlace = false;
	    break;

	case transparent:
	    for (i = 0; i < nselected; i++)
	    {
		GraphicsControlBlock gcb;

		DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
		gcb.TransparentIndex = op->color;
		EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
	    }
	    break;

	case userinput_on:
	    for (i = 0; i < nselected; i++)
	    {
		GraphicsControlBlock gcb;

		DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
		gcb.UserInputFlag = true;
		EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
	    }
	    break;

	case userinput_off:
	    for (i = 0; i < nselected; i++)
	    {
		GraphicsControlBlock gcb;

		DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
		gcb.UserInputFlag = false;
		EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
	    }
	    break;

	case disposal:
	    for (i = 0; i < nselected; i++)
	    {
		GraphicsControlBlock gcb;

		DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
		gcb.DisposalMode = op->dispose;
		EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
	    }
	    break;

	default:
	    (void)fprintf(stderr, "giftool: unknown operation mode\n");
	    exit(EXIT_FAILURE);
	}

    /* write out the results */
    GifFileOut->SWidth = GifFileIn->SWidth;
    GifFileOut->SHeight = GifFileIn->SHeight;
    GifFileOut->SColorResolution = GifFileIn->SColorResolution;
    GifFileOut->SBackGroundColor = GifFileIn->SBackGroundColor;
    GifFileOut->SColorMap = GifMakeMapObject(
				 GifFileIn->SColorMap->ColorCount,
				 GifFileIn->SColorMap->Colors);

    for (i = 0; i < GifFileIn->ImageCount; i++)
	(void) GifMakeSavedImage(GifFileOut, &GifFileIn->SavedImages[i]);

    if (EGifSpew(GifFileOut) == GIF_ERROR)
	PrintGifError();
    else if (DGifCloseFile(GifFileIn) == GIF_ERROR)
	PrintGifError();

    return 0;
}

/* giftool.c ends here */
