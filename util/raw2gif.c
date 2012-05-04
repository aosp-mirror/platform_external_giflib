/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.
*
* Written by:  Gershon Elber				Ver 0.1, Jun. 1989
******************************************************************************
* Module to convert raw image into a GIF file.
* Options:
* -v : verbose mode.
* -s Width Height : specifies size of raw image.
* -p ColorMapFile : specifies color map for ray image (see gifclrmp).
* -h : on-line help.
******************************************************************************
* History:
* 15 Oct 89 - Version 1.0 by Gershon Elber.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#ifdef __MSDOS__
#include <dos.h>
#include <graphics.h>
#include <io.h>
#endif /* __MSDOS__ */

#include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"raw2gif"

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
	" v%- s!-Width|Height!d!d p%-ColorMapFile!s h%- RawFile!*s";

static GifColorType EGAPalette[] =      /* Default color map is EGA palette. */
{
    {   0,   0,   0 },   /* 0. Black */
    {   0,   0, 170 },   /* 1. Blue */
    {   0, 170,   0 },   /* 2. Green */
    {   0, 170, 170 },   /* 3. Cyan */
    { 170,   0,   0 },   /* 4. Red */
    { 170,   0, 170 },   /* 5. Magenta */
    { 170, 170,   0 },   /* 6. Brown */
    { 170, 170, 170 },   /* 7. LightGray */
    {  85,  85,  85 },   /* 8. DarkGray */
    {  85,  85, 255 },   /* 9. LightBlue */
    {  85, 255,  85 },   /* 10. LightGreen */
    {  85, 255, 255 },   /* 11. LightCyan */
    { 255,  85,  85 },   /* 12. LightRed */
    { 255,  85, 255 },   /* 13. LightMagenta */
    { 255, 255,  85 },   /* 14. Yellow */
    { 255, 255, 255 },   /* 15. White */
};
#define EGA_PALETTE_SIZE (sizeof(EGAPalette) / sizeof(GifColorType))

int Raw2Gif(int ImagwWidth, int ImagwHeight, ColorMapObject *ColorMap);
static int HandleGifError(GifFileType *GifFile);

/******************************************************************************
* Interpret the command line, prepar global data and call the Gif routines.
******************************************************************************/
int main(int argc, char **argv)
{
    int	NumFiles, ImageWidth, ImageHeight, Dummy, Red, Green, Blue;
    static bool Error,
	ImageSizeFlag = false, ColorMapFlag = false, HelpFlag = false;
    char **FileName = NULL, *ColorMapFile;
    ColorMapObject *ColorMap;
    FILE *InColorMapFile;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GifNoisyPrint,
		&ImageSizeFlag, &ImageWidth, &ImageHeight,
		&ColorMapFlag, &ColorMapFile,
		&HelpFlag,
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

    if (ColorMapFlag) {
	int ColorMapSize;

	/* Read color map from given file: */
	if ((InColorMapFile = fopen(ColorMapFile, "rt")) == NULL) {
	    GIF_MESSAGE("Failed to open COLOR MAP file (not exists!?).");
	    exit(EXIT_FAILURE);
	}
	if ((ColorMap = MakeMapObject(256, NULL)) == NULL) {
	    GIF_MESSAGE("Failed to allocate bitmap, aborted.");
	    exit(EXIT_FAILURE);
	}

	for (ColorMapSize = 0;
	     ColorMapSize < 256 && !feof(InColorMapFile);
	     ColorMapSize++) {
	    if (fscanf(InColorMapFile, "%3d %3d %3d %3d\n",
		       &Dummy, &Red, &Green, &Blue) == 4) {
		ColorMap->Colors[ColorMapSize].Red = Red;
		ColorMap->Colors[ColorMapSize].Green = Green;
		ColorMap->Colors[ColorMapSize].Blue = Blue;
	    }
	}
    }
    else {
	ColorMap = MakeMapObject(16, EGAPalette);
    }

    if (NumFiles == 1) {
	int InFileHandle;
#ifdef __MSDOS__
	if ((InFileHandle = open(*FileName, O_RDONLY | O_BINARY)) == -1) {
#else
	if ((InFileHandle = open(*FileName, O_RDONLY)) == -1) {
#endif /* __MSDOS__ */
	    GIF_MESSAGE("Failed to open RAW image file (not exists!?).");
	    exit(EXIT_FAILURE);
	}
	dup2(InFileHandle, 0);		       /* Make stdin from this file. */
    }
    else {
#ifdef __MSDOS__
	setmode(0, O_BINARY);		  /* Make sure it is in binary mode. */
#endif /* __MSDOS__ */
    }

    setvbuf(stdin, NULL, _IOFBF, GIF_FILE_BUFFER_SIZE);

    /* Conver Raw image from stdin to Gif file in stdout: */
    Raw2Gif(ImageWidth, ImageHeight, ColorMap);

    return 0;
    // cppcheck-suppress resourceLeak
}

/******************************************************************************
* Convert Raw image (One byte per pixel) into Gif file. Raw data is read from
* stdin, and Gif is dumped to stdout. ImagwWidth times ImageHeight bytes are
* read. Color map is dumped from ColorMap.
******************************************************************************/
int Raw2Gif(int ImageWidth, int ImageHeight, ColorMapObject *ColorMap)
{
    int i, j;
    static GifPixelType *ScanLine;
    GifFileType *GifFile;

    if ((ScanLine = (GifPixelType *) malloc(sizeof(GifPixelType) * ImageWidth))
								== NULL) {
	GIF_MESSAGE("Failed to allocate scan line, aborted.");
	exit(EXIT_FAILURE);
    }

    if ((GifFile = EGifOpenFileHandle(1)) == NULL) {	   /* Gif to stdout. */
	free((char *) ScanLine);
	return HandleGifError(GifFile);
    }

    if (EGifPutScreenDesc(GifFile, ImageWidth, ImageHeight, ColorMap->BitsPerPixel,
			  0, ColorMap) == GIF_ERROR) {
	free((char *) ScanLine);
	return HandleGifError(GifFile);
    }

    if (EGifPutImageDesc(GifFile, 0, 0, ImageWidth, ImageHeight, false,
			 NULL) == GIF_ERROR) {
	free((char *) ScanLine);
	return HandleGifError(GifFile);
    }

    /* Here it is - get one raw line from stdin, and dump to stdout Gif: */
    GifQprintf("\n%s: Image 1 at (0, 0) [%dx%d]:     ",
	PROGRAM_NAME, ImageWidth, ImageHeight);
    for (i = 0; i < ImageHeight; i++) {
	/* Note we assume here PixelSize == Byte, which is not necessarily   */
	/* so. If not - must read one byte at a time, and coerce to pixel.   */
	if (fread(ScanLine, 1, ImageWidth, stdin) != (unsigned)ImageWidth) {
	    GIF_MESSAGE("RAW input file ended prematurely.");
	    exit(EXIT_FAILURE);
	}

	for (j = 0; j < ImageWidth; j++)
	    if (ScanLine[j] >= ColorMap->ColorCount)
		GIF_MESSAGE("Warning: RAW data color > maximum color map entry.");

	if (EGifPutLine(GifFile, ScanLine, ImageWidth) == GIF_ERROR) {
	    free((char *) ScanLine);
	    return HandleGifError(GifFile);
	}
	GifQprintf("\b\b\b\b%-4d", i);
    }

    if (EGifCloseFile(GifFile) == GIF_ERROR) {
	free((char *) ScanLine);
	return HandleGifError(GifFile);
    }

    free((char *) ScanLine);
    return 0;
}

/******************************************************************************
* Handle last GIF error. Try to close the file and free all allocated memory. *
******************************************************************************/
static int HandleGifError(GifFileType *GifFile)
{
    int i = GifLastError();

    if (EGifCloseFile(GifFile) == GIF_ERROR) {
	GifLastError();
    }
    return i;
}
