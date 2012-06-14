/*****************************************************************************

gifrotate - rotate a GIF image by a specified angle

*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"gifrotate"

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
	" a!-Angle!d v%- s%-Width|Height!d!d h%- GifFile!*s";

static int
    InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
    InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

static void RotateGifImage(GifRowType *ScreenBuffer, 
			   GifFileType *SrcGifFile, GifFileType *DstGifFile,
			   int Angle,
			   int DstWidth, int DstHeight);
static void RotateGifLine(GifRowType *ScreenBuffer, int BackGroundColor,
			  int SrcWidth, int SrcHeight,
			  int Angle, GifRowType DstLine,
			  int DstWidth, int DstHeight, int y);
static void QuitGifError(GifFileType *SrcGifFile, GifFileType *DstGifFile);

/******************************************************************************
* Interpret the command line and scan the given GIF file.
******************************************************************************/
int main(int argc, char **argv)
{
    int	i, j, Size, NumFiles, Col, Row, Count, ExtCode,
	DstWidth, DstHeight, Width, Height,
	ImageNum = 0, Angle = 0;
    bool Error,
	DstSizeFlag = false,
	AngleFlag = false,
	HelpFlag = false;
    char **FileName = NULL;
    GifRecordType RecordType;
    GifByteType *Extension;
    GifFileType *GifFileIn;
    GifFileType *GifFileOut;
    GifRowType *ScreenBuffer;

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
		&AngleFlag, &Angle, &GifNoisyPrint,
		&DstSizeFlag, &DstWidth, &DstHeight, &HelpFlag,
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

    if (NumFiles == 1) {
	if ((GifFileIn = DGifOpenFileName(*FileName)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }
    else {
	/* Use stdin instead */
	if ((GifFileIn = DGifOpenFileHandle(0)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }

    if (!DstSizeFlag) {
	DstWidth = GifFileIn->SWidth;
	DstHeight = GifFileIn->SHeight;
    }

    /* Open stdout for the output file: */
    if ((GifFileOut = EGifOpenFileHandle(1)) == NULL)
	QuitGifError(GifFileIn, GifFileOut);

    if (EGifPutScreenDesc(GifFileOut,
	GifFileIn->SWidth, GifFileIn->SHeight,
	GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
	GifFileIn->SColorMap) == GIF_ERROR) {
	PrintGifError();
	exit(EXIT_FAILURE);
    }

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
	if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
		/* 
		 * Allocate the screen as vector of column of rows. Note this
		 * screen is device independent - it's the screen defined by the
		 * GIF file parameters.
		 */
		if ((ScreenBuffer = (GifRowType *)
		    malloc(GifFileIn->SHeight * sizeof(GifRowType))) == NULL)
			GIF_EXIT("Failed to alloc memory required, aborted.");
		/* Size in bytes of one row.*/
		Size = GifFileIn->SWidth * sizeof(GifPixelType);
		 /* First row. */
		if ((ScreenBuffer[0] = (GifRowType) malloc(Size)) == NULL)
		    GIF_EXIT("Failed to allocate memory required, aborted.");
		  /* Set its color to BackGround. */
		for (i = 0; i < GifFileIn->SWidth; i++)
		    ScreenBuffer[0][i] = GifFileIn->SBackGroundColor;
		/* Allocate other rows, and set their color to background too */
		for (i = 1; i < GifFileIn->SHeight; i++) {
		    if ((ScreenBuffer[i] = (GifRowType) malloc(Size)) == NULL)
			GIF_EXIT("Failed to alloc memory required, aborted.");
		    memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
		}
		if (DGifGetImageDesc(GifFileIn) == GIF_ERROR) {
		    PrintGifError();
		    exit(EXIT_FAILURE);
		}
		Row = GifFileIn->Image.Top; /* Image Position relative to Screen. */
		Col = GifFileIn->Image.Left;
		Width = GifFileIn->Image.Width;
		Height = GifFileIn->Image.Height;
		GifQprintf("\n%s: Image %d at (%d, %d) [%dx%d]:     ",
		    PROGRAM_NAME, ++ImageNum, Col, Row, Width, Height);
		if (GifFileIn->Image.Left + GifFileIn->Image.Width > GifFileIn->SWidth ||
		   GifFileIn->Image.Top + GifFileIn->Image.Height > GifFileIn->SHeight) {
		    fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n",ImageNum);
		    exit(EXIT_FAILURE);
		}
		if (GifFileIn->Image.Interlace) {
		    /* Need to perform 4 passes on the images: */
		    for (Count = i = 0; i < 4; i++)
			for (j = Row + InterlacedOffset[i]; j < Row + Height;
						 j += InterlacedJumps[i]) {
			    GifQprintf("\b\b\b\b%-4d", Count++);
			    if (DGifGetLine(GifFileIn, &ScreenBuffer[j][Col],
				Width) == GIF_ERROR) {
				PrintGifError();
				exit(EXIT_FAILURE);
			    }
			}
		}
		else {
		    for (i = 0; i < Height; i++) {
			GifQprintf("\b\b\b\b%-4d", i);
			if (DGifGetLine(GifFileIn, &ScreenBuffer[Row++][Col],
				Width) == GIF_ERROR) {
			    PrintGifError();
			    exit(EXIT_FAILURE);
			}
		    }
		}
		/* Perform the actual rotation and dump the image: */
		RotateGifImage(ScreenBuffer, GifFileIn, GifFileOut,
			       Angle, DstWidth, DstHeight);
		(void)free(ScreenBuffer);
		break;
	    case EXTENSION_RECORD_TYPE:
		/* pass through extension records */
		if (DGifGetExtension(GifFileIn, &ExtCode, &Extension) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		if (EGifPutExtensionLeader(GifFileOut, ExtCode) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		if (EGifPutExtensionBlock(GifFileOut, 
					  Extension[0],
					  Extension + 1) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		while (Extension != NULL) {
		    if (DGifGetExtensionNext(GifFileIn, &Extension)==GIF_ERROR)
			QuitGifError(GifFileIn, GifFileOut);
		    if (Extension != NULL)
			if (EGifPutExtensionBlock(GifFileOut, 
						  Extension[0],
						  Extension + 1) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
		}
		if (EGifPutExtensionTrailer(GifFileOut) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		break;
	    case TERMINATE_RECORD_TYPE:
		if (EGifCloseFile(GifFileOut) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);
		break;
	    default:		    /* Should be traps by DGifGetRecordType. */
		break;
	}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

    return 0;
}

/******************************************************************************
* Save the GIF resulting image.
******************************************************************************/
static void RotateGifImage(GifRowType *ScreenBuffer, 
			   GifFileType *SrcGifFile,
			   GifFileType *DstGifFile,
			   int Angle,
			   int DstWidth, int DstHeight)
{
    int i,
	LineSize = DstWidth * sizeof(GifPixelType);
    GifRowType LineBuffer;

    if ((LineBuffer = (GifRowType) malloc(LineSize)) == NULL)
	GIF_EXIT("Failed to allocate memory required, aborted.");

    if (EGifPutImageDesc(DstGifFile, 0, 0, DstWidth, DstHeight,
			 false, NULL) == GIF_ERROR)
	QuitGifError(SrcGifFile, DstGifFile);

    for (i = 0; i < DstHeight; i++) {
	RotateGifLine(ScreenBuffer, SrcGifFile->SBackGroundColor,
		      SrcGifFile->SWidth, SrcGifFile->SHeight,
		      Angle, LineBuffer, DstWidth, DstHeight, i);
	if (EGifPutLine(DstGifFile, LineBuffer, DstWidth) == GIF_ERROR)
	    QuitGifError(SrcGifFile, DstGifFile);
	GifQprintf("\b\b\b\b%-4d", DstHeight - i - 1);
    }
}


/******************************************************************************
* Save the GIF resulting image.
******************************************************************************/
static void RotateGifLine(GifRowType *ScreenBuffer, int BackGroundColor,
		          int SrcWidth, int SrcHeight,
			  int Angle, GifRowType DstLine,
			  int DstWidth, int DstHeight, int y)
{
    int x,
	TransSrcX = SrcWidth / 2,
	TransSrcY = SrcHeight / 2,
	TransDstX = DstWidth / 2,
	TransDstY = DstHeight / 2;
    double SinAngle = sin(Angle * M_PI / 180.0),
	   CosAngle = cos(Angle * M_PI / 180.0);

    for (x = 0; x < DstWidth; x++)
    {
	int xc = x - TransDstX,
	    yc = y - TransDstY,
	    SrcX = xc * CosAngle - yc * SinAngle + TransSrcX,
	    SrcY = xc * SinAngle + yc * CosAngle + TransSrcY;

	if (SrcX < 0 || SrcX >= SrcWidth ||
	    SrcY < 0 || SrcY >= SrcHeight)
	{
	    /* Out of the source image domain - set it to background color. */
	    *DstLine++ = BackGroundColor;
	}
	else
	    *DstLine++ = ScreenBuffer[SrcY][SrcX];
    }
}

/******************************************************************************
* Close both input and output file (if open), and exit.
******************************************************************************/
static void QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut)
{
    PrintGifError();
    if (GifFileIn != NULL) DGifCloseFile(GifFileIn);
    if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
    exit(EXIT_FAILURE);
}

