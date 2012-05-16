/*****************************************************************************
* "Gifinfo"
*
* Written by:  Martin Edlman			Ver 0.1.0, Jul. 1989
******************************************************************************
* Program to display information (size and comments) in GIF file.
* uses giflib on Linux. Not tried on other systems.
* Options:
* -h : on-line help.
* -f : format output string
*      \h - height
*      \w - width
*      \c - comment
*      \f - file name
*      Example: -f 'Image \f:\n Size: \wx\h\n Comment: \c\n'
******************************************************************************
* History:
* 21 Jun 99 - Version 1.0 by Martin Edlman.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"gifinfo"

#define DEFAULTFORMAT   "Size: \\wx\\h\\nComment: \\c\\n"

static char
    *VersionStr =
	PROGRAM_NAME
	VERSION_COOKIE
	"	Martin Edlman,	"
	__DATE__ ",   " __TIME__ "\n"
	"(C) Copyright 1999 Martin Edlman.\n";
static char
    *CtrlStr =
	PROGRAM_NAME
	" f%-Format!s h%- GifFile!*s";

/* Make some variables global, so we could access them faster: */
static bool
    HelpFlag = false;

/******************************************************************************
* Interpret the command line and scan the given GIF file.
******************************************************************************/
int main(int argc, char **argv)
{
    int	i, NumFiles, NumFormats, Size, Width=0, Height=0, ExtCode;
    bool Error;
    GifRecordType RecordType;
    GifByteType  *Extension;
    char         *Format = NULL, *Comment, *NewComment;
    char        **FileName = NULL;
    GifRowType    RowBuffer;
    GifFileType  *GifFile;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &NumFormats, &Format,
                           &HelpFlag, &NumFiles, &FileName)) != false ||
        (NumFiles > 1 && !HelpFlag)) {
        if (Error)
            GAPrintErrMsg(Error);
	else if (NumFiles > 1)
	    GIF_MESSAGE("Error in command line parsing - one GIF file please.");
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (Format == NULL) {
        Format = malloc(strlen(DEFAULTFORMAT)+1);
        strcpy(Format, DEFAULTFORMAT);
    }

    if (HelpFlag) {
	(void)fprintf(stderr, VersionStr, GIFLIB_MAJOR, GIFLIB_MINOR);
	GAPrintHowTo(CtrlStr);
	printf("Format: quoted text string used for formating of information\n");
	printf("  Special characters for various information can be used\n");
	printf("  \\f - filename of GIF file\n");
	printf("  \\c - comment\n");
	printf("  \\w - width of image\n");
	printf("  \\h - height of image\n");
	printf("  \\n - new line\n");
	printf("  \\\\ - backslash\n");
	printf("Default format will be used if not specified with -f:\n");
	printf("  %s\n\n", DEFAULTFORMAT);
	exit(EXIT_SUCCESS);
    }
    
    if (NumFiles == 1) {
	if ((GifFile = DGifOpenFileName(*FileName)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }
    else {
	/* Use the stdin instead: */

	if ((GifFile = DGifOpenFileHandle(0)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }

    Comment = (char*) malloc(1);
    strcpy(Comment, "\0");

    /* Allocate memory or one row which will be used as trash during reading
       image*/
    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((RowBuffer = (GifRowType) malloc(Size)) == NULL) /* First row. */
	GIF_EXIT("Failed to allocate memory required, aborted.");

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
	if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
		if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
		    PrintGifError();
		    exit(EXIT_FAILURE);
		}
		Width = GifFile->Image.Width;
		Height = GifFile->Image.Height;
		if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
		   GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
		    fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n", GifFile->ImageCount);
		    exit(EXIT_FAILURE);
		}
                /* We don't care about Interlaced as image is not interesting, read sequentialy */
		for (i = 0; i < Height; i++) {
		    GifQprintf("\b\b\b\b%-4d", i);
		    if (DGifGetLine(GifFile, &RowBuffer[0], Width) == GIF_ERROR) {
			PrintGifError();
			exit(EXIT_FAILURE);
		    }
		}
		break;
	    case EXTENSION_RECORD_TYPE:
		/* Skip any extension blocks in file except comments: */
		if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
		    PrintGifError();
		    exit(EXIT_FAILURE);
		}
		while (Extension != NULL) {
		    if(ExtCode == COMMENT_EXT_FUNC_CODE) {
			Extension[Extension[0]+1] = '\000';   /* Convert gif's pascal-like string */
			NewComment = (char*) realloc(Comment, strlen(Comment) + Extension[0] + 1);
			if (NewComment == NULL) {
			    (void) free(Comment);
			    exit(EXIT_FAILURE);
			}
			else
			    Comment = NewComment;
			strcat(Comment, (char*)Extension+1);
		    }
		    if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
			PrintGifError();
			exit(EXIT_FAILURE);
		    }
		}
		break;
	    case TERMINATE_RECORD_TYPE:
		break;
	    default:		    /* Should be traps by DGifGetRecordType. */
		break;
	}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

    while (Format[0] != '\0') {
        if (Format[0] == '\\') {
            Format++;
            switch (Format[0]) {
            case 'w': printf("%i",  Width);     break;
            case 'h': printf("%i",  Height);    break;
            case 'c': printf("%s",  Comment);   break;
            case 'f': printf("%s", *FileName);  break;
            case 'n': printf("\n");             break;
            default:  printf("%c",  Format[0]); break;
            }
         }
         else {
            printf("%c", Format[0]);
         }
         Format++;
    }

    if (DGifCloseFile(GifFile) == GIF_ERROR) {
	PrintGifError();
	exit(EXIT_FAILURE);
    }

    // cppcheck-suppress memleak
    return 0;
}
