/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.
*
* Written by:  Gershon Elber				Ver 0.1, Jun. 1991
******************************************************************************
* Program to convert 24bits RGB files to GIF format.
* Options:
* -v : verbose mode.
* -c #colors : in power of two, i.e. 7 will allow up to 128 colors in output.
* -1 : one file holding RGBRGB.. triples of bytes
* -s Width Height : specifies size of raw image.
* -h : on-line help.
******************************************************************************
* History:
* 15 Jun 91 - Version 1.0 by Gershon Elber.
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
 * Code to quantize high resolution image into lower one. You may want to
 * peek into the following article this code is based on:
 * "Color Image Quantization for frame buffer Display", by Paul Heckbert
 * SIGGRAPH 1982 page 297-307.
 ******************************************************************************
 * History:
 * 5 Jan 90 - Version 1.0 by Gershon Elber.
 *****************************************************************************/

#define ABS(x)    ((x) > 0 ? (x) : (-(x)))

#define COLOR_ARRAY_SIZE 32768
#define BITS_PER_PRIM_COLOR 5
#define MAX_PRIM_COLOR      0x1f

static int SortRGBAxis;

typedef struct QuantizedColorType {
    GifByteType RGB[3];
    GifByteType NewColorIndex;
    long Count;
    struct QuantizedColorType *Pnext;
} QuantizedColorType;

typedef struct NewColorMapType {
    GifByteType RGBMin[3], RGBWidth[3];
    unsigned int NumEntries; /* # of QuantizedColorType in linked list below */
    unsigned long Count; /* Total number of pixels in all the entries */
    QuantizedColorType *QuantizedColors;
} NewColorMapType;

static int SubdivColorMap(NewColorMapType * NewColorSubdiv,
                          unsigned int ColorMapSize,
                          unsigned int *NewColorMapSize);
static int SortCmpRtn(const void *Entry1, const void *Entry2);

/******************************************************************************
 * Quantize high resolution image into lower one. Input image consists of a
 * 2D array for each of the RGB colors with size Width by Height. There is no
 * Color map for the input. Output is a quantized image with 2D array of
 * indexes into the output color map.
 *   Note input image can be 24 bits at the most (8 for red/green/blue) and
 * the output has 256 colors at the most (256 entries in the color map.).
 * ColorMapSize specifies size of color map up to 256 and will be updated to
 * real size before returning.
 *   Also non of the parameter are allocated by this routine.
 *   This function returns GIF_OK if succesfull, GIF_ERROR otherwise.
 ******************************************************************************/
static int
QuantizeBuffer(unsigned int Width,
               unsigned int Height,
               int *ColorMapSize,
               GifByteType * RedInput,
               GifByteType * GreenInput,
               GifByteType * BlueInput,
               GifByteType * OutputBuffer,
               GifColorType * OutputColorMap) {

    unsigned int Index, NumOfEntries;
    int i, j, MaxRGBError[3];
    unsigned int NewColorMapSize;
    long Red, Green, Blue;
    NewColorMapType NewColorSubdiv[256];
    QuantizedColorType *ColorArrayEntries, *QuantizedColor;

    ColorArrayEntries = (QuantizedColorType *)malloc(
                           sizeof(QuantizedColorType) * COLOR_ARRAY_SIZE);
    if (ColorArrayEntries == NULL) {
        return GIF_ERROR;
    }

    for (i = 0; i < COLOR_ARRAY_SIZE; i++) {
        ColorArrayEntries[i].RGB[0] = i >> (2 * BITS_PER_PRIM_COLOR);
        ColorArrayEntries[i].RGB[1] = (i >> BITS_PER_PRIM_COLOR) &
           MAX_PRIM_COLOR;
        ColorArrayEntries[i].RGB[2] = i & MAX_PRIM_COLOR;
        ColorArrayEntries[i].Count = 0;
    }

    /* Sample the colors and their distribution: */
    for (i = 0; i < (int)(Width * Height); i++) {
        Index = ((RedInput[i] >> (8 - BITS_PER_PRIM_COLOR)) <<
                  (2 * BITS_PER_PRIM_COLOR)) +
                ((GreenInput[i] >> (8 - BITS_PER_PRIM_COLOR)) <<
                  BITS_PER_PRIM_COLOR) +
                (BlueInput[i] >> (8 - BITS_PER_PRIM_COLOR));
        ColorArrayEntries[Index].Count++;
    }

    /* Put all the colors in the first entry of the color map, and call the
     * recursive subdivision process.  */
    for (i = 0; i < 256; i++) {
        NewColorSubdiv[i].QuantizedColors = NULL;
        NewColorSubdiv[i].Count = NewColorSubdiv[i].NumEntries = 0;
        for (j = 0; j < 3; j++) {
            NewColorSubdiv[i].RGBMin[j] = 0;
            NewColorSubdiv[i].RGBWidth[j] = 255;
        }
    }

    /* Find the non empty entries in the color table and chain them: */
    for (i = 0; i < COLOR_ARRAY_SIZE; i++)
        if (ColorArrayEntries[i].Count > 0)
            break;
    QuantizedColor = NewColorSubdiv[0].QuantizedColors = &ColorArrayEntries[i];
    NumOfEntries = 1;
    while (++i < COLOR_ARRAY_SIZE)
        if (ColorArrayEntries[i].Count > 0) {
            QuantizedColor->Pnext = &ColorArrayEntries[i];
            QuantizedColor = &ColorArrayEntries[i];
            NumOfEntries++;
        }
    QuantizedColor->Pnext = NULL;

    NewColorSubdiv[0].NumEntries = NumOfEntries; /* Different sampled colors */
    NewColorSubdiv[0].Count = ((long)Width) * Height; /* Pixels */
    NewColorMapSize = 1;
    if (SubdivColorMap(NewColorSubdiv, *ColorMapSize, &NewColorMapSize) !=
       GIF_OK) {
        free((char *)ColorArrayEntries);
        return GIF_ERROR;
    }
    if (NewColorMapSize < *ColorMapSize) {
        /* And clear rest of color map: */
        for (i = NewColorMapSize; i < *ColorMapSize; i++)
            OutputColorMap[i].Red = OutputColorMap[i].Green =
                OutputColorMap[i].Blue = 0;
    }

    /* Average the colors in each entry to be the color to be used in the
     * output color map, and plug it into the output color map itself. */
    for (i = 0; i < NewColorMapSize; i++) {
        if ((j = NewColorSubdiv[i].NumEntries) > 0) {
            QuantizedColor = NewColorSubdiv[i].QuantizedColors;
            Red = Green = Blue = 0;
            while (QuantizedColor) {
                QuantizedColor->NewColorIndex = i;
                Red += QuantizedColor->RGB[0];
                Green += QuantizedColor->RGB[1];
                Blue += QuantizedColor->RGB[2];
                QuantizedColor = QuantizedColor->Pnext;
            }
            OutputColorMap[i].Red = (Red << (8 - BITS_PER_PRIM_COLOR)) / j;
            OutputColorMap[i].Green = (Green << (8 - BITS_PER_PRIM_COLOR)) / j;
            OutputColorMap[i].Blue = (Blue << (8 - BITS_PER_PRIM_COLOR)) / j;
        } else
            fprintf(stderr,
                    "\n%s: Null entry in quantized color map - that's weird.\n",
                    PROGRAM_NAME);
    }

    /* Finally scan the input buffer again and put the mapped index in the
     * output buffer.  */
    MaxRGBError[0] = MaxRGBError[1] = MaxRGBError[2] = 0;
    for (i = 0; i < (int)(Width * Height); i++) {
        Index = ((RedInput[i] >> (8 - BITS_PER_PRIM_COLOR)) <<
                 (2 * BITS_PER_PRIM_COLOR)) +
                ((GreenInput[i] >> (8 - BITS_PER_PRIM_COLOR)) <<
                 BITS_PER_PRIM_COLOR) +
                (BlueInput[i] >> (8 - BITS_PER_PRIM_COLOR));
        Index = ColorArrayEntries[Index].NewColorIndex;
        OutputBuffer[i] = Index;
        if (MaxRGBError[0] < ABS(OutputColorMap[Index].Red - RedInput[i]))
            MaxRGBError[0] = ABS(OutputColorMap[Index].Red - RedInput[i]);
        if (MaxRGBError[1] < ABS(OutputColorMap[Index].Green - GreenInput[i]))
            MaxRGBError[1] = ABS(OutputColorMap[Index].Green - GreenInput[i]);
        if (MaxRGBError[2] < ABS(OutputColorMap[Index].Blue - BlueInput[i]))
            MaxRGBError[2] = ABS(OutputColorMap[Index].Blue - BlueInput[i]);
    }

#ifdef DEBUG
    fprintf(stderr,
            "Quantization L(0) errors: Red = %d, Green = %d, Blue = %d.\n",
            MaxRGBError[0], MaxRGBError[1], MaxRGBError[2]);
#endif /* DEBUG */

    free((char *)ColorArrayEntries);

    *ColorMapSize = NewColorMapSize;

    return GIF_OK;
}

/******************************************************************************
 * Routine to subdivide the RGB space recursively using median cut in each
 * axes alternatingly until ColorMapSize different cubes exists.
 * The biggest cube in one dimension is subdivide unless it has only one entry.
 * Returns GIF_ERROR if failed, otherwise GIF_OK.
 ******************************************************************************/
static int
SubdivColorMap(NewColorMapType * NewColorSubdiv,
               unsigned int ColorMapSize,
               unsigned int *NewColorMapSize) {

    int MaxSize;
    unsigned int i, j, Index = 0, NumEntries, MinColor, MaxColor;
    long Sum, Count;
    QuantizedColorType *QuantizedColor, **SortArray;

    while (ColorMapSize > *NewColorMapSize) {
        /* Find candidate for subdivision: */
        MaxSize = -1;
        for (i = 0; i < *NewColorMapSize; i++) {
            for (j = 0; j < 3; j++) {
                if ((((int)NewColorSubdiv[i].RGBWidth[j]) > MaxSize) &&
                      (NewColorSubdiv[i].NumEntries > 1)) {
                    MaxSize = NewColorSubdiv[i].RGBWidth[j];
                    Index = i;
                    SortRGBAxis = j;
                }
            }
        }

        if (MaxSize == -1)
            return GIF_OK;

        /* Split the entry Index into two along the axis SortRGBAxis: */

        /* Sort all elements in that entry along the given axis and split at
         * the median.  */
        SortArray = (QuantizedColorType **)malloc(
                      sizeof(QuantizedColorType *) * 
                      NewColorSubdiv[Index].NumEntries);
        if (SortArray == NULL)
            return GIF_ERROR;
        for (j = 0, QuantizedColor = NewColorSubdiv[Index].QuantizedColors;
             j < NewColorSubdiv[Index].NumEntries && QuantizedColor != NULL;
             j++, QuantizedColor = QuantizedColor->Pnext)
            SortArray[j] = QuantizedColor;

        qsort(SortArray, NewColorSubdiv[Index].NumEntries,
              sizeof(QuantizedColorType *), SortCmpRtn);

        /* Relink the sorted list into one: */
        for (j = 0; j < NewColorSubdiv[Index].NumEntries - 1; j++)
            SortArray[j]->Pnext = SortArray[j + 1];
        SortArray[NewColorSubdiv[Index].NumEntries - 1]->Pnext = NULL;
        NewColorSubdiv[Index].QuantizedColors = QuantizedColor = SortArray[0];
        free((char *)SortArray);

        /* Now simply add the Counts until we have half of the Count: */
        Sum = NewColorSubdiv[Index].Count / 2 - QuantizedColor->Count;
        NumEntries = 1;
        Count = QuantizedColor->Count;
        while (QuantizedColor->Pnext != NULL &&
	       (Sum -= QuantizedColor->Pnext->Count) >= 0 &&
               QuantizedColor->Pnext->Pnext != NULL) {
            QuantizedColor = QuantizedColor->Pnext;
            NumEntries++;
            Count += QuantizedColor->Count;
        }
        /* Save the values of the last color of the first half, and first
         * of the second half so we can update the Bounding Boxes later.
         * Also as the colors are quantized and the BBoxes are full 0..255,
         * they need to be rescaled.
         */
        MaxColor = QuantizedColor->RGB[SortRGBAxis]; /* Max. of first half */
	/* coverity[var_deref_op] */
        MinColor = QuantizedColor->Pnext->RGB[SortRGBAxis]; /* of second */
        MaxColor <<= (8 - BITS_PER_PRIM_COLOR);
        MinColor <<= (8 - BITS_PER_PRIM_COLOR);

        /* Partition right here: */
        NewColorSubdiv[*NewColorMapSize].QuantizedColors =
           QuantizedColor->Pnext;
        QuantizedColor->Pnext = NULL;
        NewColorSubdiv[*NewColorMapSize].Count = Count;
        NewColorSubdiv[Index].Count -= Count;
        NewColorSubdiv[*NewColorMapSize].NumEntries =
           NewColorSubdiv[Index].NumEntries - NumEntries;
        NewColorSubdiv[Index].NumEntries = NumEntries;
        for (j = 0; j < 3; j++) {
            NewColorSubdiv[*NewColorMapSize].RGBMin[j] =
               NewColorSubdiv[Index].RGBMin[j];
            NewColorSubdiv[*NewColorMapSize].RGBWidth[j] =
               NewColorSubdiv[Index].RGBWidth[j];
        }
        NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] =
           NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] +
           NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] - MinColor;
        NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] = MinColor;

        NewColorSubdiv[Index].RGBWidth[SortRGBAxis] =
           MaxColor - NewColorSubdiv[Index].RGBMin[SortRGBAxis];

        (*NewColorMapSize)++;
    }

    return GIF_OK;
}

/****************************************************************************
 * Routine called by qsort to compare two entries.
 ****************************************************************************/
static int
SortCmpRtn(const void *Entry1,
           const void *Entry2) {

    return (*((QuantizedColorType **) Entry1))->RGB[SortRGBAxis] -
       (*((QuantizedColorType **) Entry2))->RGB[SortRGBAxis];
}
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

    if (QuantizeBuffer(Width, Height, &ColorMapSize,
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
