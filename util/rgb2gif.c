/*****************************************************************************

raw2gif - convert raw pixel data to a GIF

*****************************************************************************/

/***************************************************************************

Toshio Kuratomi had written this:

  Besides fixing bugs, what's really needed is for someone to work out how to
  calculate a colormap for writing gifs from rgb sources.  Right now, an rgb
  source that has only two colors (b/w) is being converted into an 8 bit gif....
  Which is horrendously wasteful without compression.

I (ESR) took this off the main to-do list in 2012 because I don't think
the GIFLIB project actually needs to be in the converters-and-tools business.
Plenty of hackers do that; our jub is to supply stable library capability 
with our utilities mainly interesting as test tools.

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#ifdef __MSDOS__
#include <io.h>
#endif /* __MSDOS__ */

#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"rgb2gif"

static char
    *VersionStr =
	PROGRAM_NAME
	VERSION_COOKIE
	"	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n"
	"(C) Copyright 1989 Gershon Elber.\n";
static char
    *CtrlStr =
	PROGRAM_NAME
	" v%- c%-#Colors!d 1%- s!-Width|Height!d!d h%- RGBFile!*s";

/* Make some variables global, so we could access them faster: */
static int
    ExpNumOfColors = 8,
    ColorMapSize = 256;
static bool
    ColorFlag = false,
    OneFileFlag = false,
    HelpFlag = false;

static void LoadRGB(char *FileName,
		    int OneFileFlag,
		    GifByteType **RedBuffer,
		    GifByteType **GreenBuffer,
		    GifByteType **BlueBuffer,
		    int Width, int Height);
static void SaveGif(GifByteType *OutputBuffer,
		    ColorMapObject *OutputColorMap,
		    int ExpColorMapSize, int Width, int Height);
static void QuitGifError(GifFileType *GifFile);

/******************************************************************************
* Interpret the command line and scan the given GIF file.
******************************************************************************/
int main(int argc, char **argv)
{
    int	NumFiles, Width, Height, SizeFlag;
    bool Error;
    char **FileName = NULL;
    GifByteType *RedBuffer = NULL, *GreenBuffer = NULL, *BlueBuffer = NULL,
	*OutputBuffer = NULL;
    ColorMapObject *OutputColorMap = NULL;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GifNoisyPrint,
		&ColorFlag, &ExpNumOfColors, &OneFileFlag,
		&SizeFlag, &Width, &Height, &HelpFlag,
		&NumFiles, &FileName)) != false ||
		(NumFiles > 1 && !HelpFlag)) {
	if (Error)
	    GAPrintErrMsg(Error);
	else if (NumFiles > 1)
	    GIF_MESSAGE("Error in command line parsing - one GIF file please.");
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (HelpFlag) {
	(void)fprintf(stderr, VersionStr, GIFLIB_MAJOR, GIFLIB_MINOR);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_SUCCESS);
    }

    ColorMapSize = 1 << ExpNumOfColors;

    if (NumFiles == 1) {
	LoadRGB(*FileName, OneFileFlag,
		&RedBuffer, &GreenBuffer, &BlueBuffer, Width, Height);
    }
    else {
	LoadRGB(NULL, OneFileFlag,
		&RedBuffer, &GreenBuffer, &BlueBuffer, Width, Height);
    }

    if ((OutputColorMap = MakeMapObject(ColorMapSize, NULL)) == NULL ||
	(OutputBuffer = (GifByteType *) malloc(Width * Height *
					    sizeof(GifByteType))) == NULL)
	GIF_EXIT("Failed to allocate memory required, aborted.");

    if (GifQuantizeBuffer(Width, Height, &ColorMapSize,
		       RedBuffer, GreenBuffer, BlueBuffer,
		       OutputBuffer, OutputColorMap->Colors) == GIF_ERROR)
	QuitGifError(NULL);
    free((char *) RedBuffer);
    free((char *) GreenBuffer);
    free((char *) BlueBuffer);

    SaveGif(OutputBuffer, OutputColorMap, ExpNumOfColors, Width, Height);

    return 0;
}

/******************************************************************************
* Load RGB file into internal frame buffer.
******************************************************************************/
static void LoadRGB(char *FileName,
		    int OneFileFlag,
		    GifByteType **RedBuffer,
		    GifByteType **GreenBuffer,
		    GifByteType **BlueBuffer,
		    int Width, int Height)
{
    int i;
    unsigned long Size;
    GifByteType *RedP, *GreenP, *BlueP;
    FILE *f[3];

    Size = ((long) Width) * Height * sizeof(GifByteType);

    if ((*RedBuffer = (GifByteType *) malloc((unsigned int) Size)) == NULL ||
	(*GreenBuffer = (GifByteType *) malloc((unsigned int) Size)) == NULL ||
	(*BlueBuffer = (GifByteType *) malloc((unsigned int) Size)) == NULL)
	GIF_EXIT("Failed to allocate memory required, aborted.");

    RedP = *RedBuffer;
    GreenP = *GreenBuffer;
    BlueP = *BlueBuffer;

    if (FileName != NULL) {
	char OneFileName[80];

	if (OneFileFlag) {
	    if ((f[0] = fopen(FileName, "rb")) == NULL)
		GIF_EXIT("Can't open input file name.");
	}
	else {
	    static char *Postfixes[] = { ".R", ".G", ".B" };

	    for (i = 0; i < 3; i++) {
		strncpy(OneFileName, FileName, sizeof(OneFileName)-1);
		/* cppcheck-suppress uninitstring */
		strncat(OneFileName, Postfixes[i], 
			sizeof(OneFileName) - 1 - strlen(OneFileName));

		if ((f[i] = fopen(OneFileName, "rb")) == NULL)
		    GIF_EXIT("Can't open input file name.");
	    }
	}
    }
    else {
	OneFileFlag = true;

#ifdef __MSDOS__
	setmode(0, O_BINARY);
#endif /* __MSDOS__ */

	f[0] = stdin;
    }

    GifQprintf("\n%s: RGB image:     ", PROGRAM_NAME);

    if (OneFileFlag) {
	GifByteType *Buffer, *BufferP;

	if ((Buffer = (GifByteType *) malloc(Width * 3)) == NULL)
	    GIF_EXIT("Failed to allocate memory required, aborted.");

	for (i = 0; i < Height; i++) {
	    int j;
	    GifQprintf("\b\b\b\b%-4d", i);
	    if (fread(Buffer, Width * 3, 1, f[0]) != 1)
		GIF_EXIT("Input file(s) terminated prematurly.");
	    for (j = 0, BufferP = Buffer; j < Width; j++) {
		*RedP++ = *BufferP++;
		*GreenP++ = *BufferP++;
		*BlueP++ = *BufferP++;
	    }
	}

	free((char *) Buffer);
	fclose(f[0]);
    }
    else {
	for (i = 0; i < Height; i++) {
	    GifQprintf("\b\b\b\b%-4d", i);
	    if (fread(RedP, Width, 1, f[0]) != 1 ||
		fread(GreenP, Width, 1, f[1]) != 1 ||
		fread(BlueP, Width, 1, f[2]) != 1)
		GIF_EXIT("Input file(s) terminated prematurly.");
	    RedP += Width;
	    GreenP += Width;
	    BlueP += Width;
	}

	fclose(f[0]);
	fclose(f[1]);
	fclose(f[2]);
    }
}

/******************************************************************************
* Save the GIF resulting image.
******************************************************************************/
static void SaveGif(GifByteType *OutputBuffer,
		    ColorMapObject *OutputColorMap,
		    int ExpColorMapSize, int Width, int Height)
{
    int i;
    GifFileType *GifFile;
    GifByteType *Ptr = OutputBuffer;

    /* Open stdout for the output file: */
    if ((GifFile = EGifOpenFileHandle(1)) == NULL)
	QuitGifError(GifFile);

    if (EGifPutScreenDesc(GifFile,
			  Width, Height, ExpColorMapSize, 0,
			  OutputColorMap) == GIF_ERROR ||
	EGifPutImageDesc(GifFile,
			 0, 0, Width, Height, false, NULL) ==
	                                                             GIF_ERROR)
	QuitGifError(GifFile);

    GifQprintf("\n%s: Image 1 at (%d, %d) [%dx%d]:     ",
	       PROGRAM_NAME, GifFile->Image.Left, GifFile->Image.Top,
	       GifFile->Image.Width, GifFile->Image.Height);

    for (i = 0; i < Height; i++) {
	if (EGifPutLine(GifFile, Ptr, Width) == GIF_ERROR)
	    QuitGifError(GifFile);
	GifQprintf("\b\b\b\b%-4d", Height - i - 1);

	Ptr += Width;
    }

    if (EGifCloseFile(GifFile) == GIF_ERROR)
	QuitGifError(GifFile);
}

/******************************************************************************
* Close output file (if open), and exit.
******************************************************************************/
static void QuitGifError(GifFileType *GifFile)
{
    PrintGifError();
    if (GifFile != NULL) EGifCloseFile(GifFile);
    exit(EXIT_FAILURE);
}
