/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.
*
* Written by:  Gershon Elber				Ver 0.1, Jun. 1989
******************************************************************************
* Module to convert an editable text representation into a GIF file.
******************************************************************************
* History:
* 15 Sep 92 - Version 1.0 by Eric Raymond.
*****************************************************************************/

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"icon2gif"

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
	" v%- d%- t%-Characters!s h%- GifFile(s)!*s";

static char KeyLetters[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:<=>?@[\\]^_`{|}~";
#define PRINTABLES	(sizeof(KeyLetters) - 1)

static void Icon2Gif(char *FileName, FILE *txtin, int fdout);
static void Gif2Icon(char *FileName,
		     int fdin, int fdout,
		     char NameTable[]);
static void VisibleDumpBuffer(char *buf, int Len);
static int EscapeString(char *cp, char *tp);

/******************************************************************************
* Main Sequence
******************************************************************************/
int main(int argc, char **argv)
{
    int	NumFiles;
    bool Error,	DisasmFlag = false, HelpFlag = false, TextLineFlag = false;
    char **FileNames = NULL;
    char *TextLines[1];

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
		&GifNoisyPrint, &DisasmFlag, &TextLineFlag, &TextLines[0],
		&HelpFlag, &NumFiles, &FileNames)) != false) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (HelpFlag) {
	(void)fprintf(stderr, VersionStr, GIFLIB_MAJOR, GIFLIB_MINOR);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_SUCCESS);
    }

    if (!DisasmFlag && NumFiles > 1) {
	GIF_MESSAGE("Error in command line parsing - one  text input please.");
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (!DisasmFlag && TextLineFlag) {
	GIF_MESSAGE("Error in command line parsing - -t invalid without -d.");
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }


    if (NumFiles == 0)
    {
	if (DisasmFlag)
	    Gif2Icon("Stdin", 0, 1, TextLineFlag ? TextLines[0] : KeyLetters);
	else
	    Icon2Gif("Stdin", stdin, 1);
    }
    else 
    {
	int i;
	for (i = 0; i < NumFiles; i++)
	{
	    FILE	*fp;

	    if ((fp = fopen(FileNames[i], "r")) == (FILE *)NULL)
	    {
		(void) fprintf(stderr, "Can't open %s\n", FileNames[i]);
		exit(EXIT_FAILURE);
	    }

	    if (DisasmFlag)
	    {
		printf("#\n# GIF information from %s\n", FileNames[i]);
		Gif2Icon(FileNames[i], -1, 1, TextLineFlag ? TextLines[0] : KeyLetters);
	    }
	    else
	    {
		Icon2Gif(FileNames[i], fp, 1);
	    }

	    (void) fclose(fp);
	}
    }

    return 0;
}

/******************************************************************************
* Parse image directives
******************************************************************************/
#define PARSE_ERROR(str)  (void) fprintf(stderr,"%s:%d: %s\n",FileName,LineNum,str);

static void Icon2Gif(char *FileName, FILE *txtin, int fdout)
{
    unsigned int	ColorMapSize = 0;
    GifColorType GlobalColorMap[256], LocalColorMap[256],
	*ColorMap = GlobalColorMap;
    char GlobalColorKeys[PRINTABLES], LocalColorKeys[PRINTABLES],
	*KeyTable = GlobalColorKeys;
#ifndef __COVERITY__
    unsigned int ExtCode;
    int red, green, blue, n;
#endif /* __COVERITY__ */

    char buf[BUFSIZ * 2], InclusionFile[64];
    GifFileType *GifFileOut;
    SavedImage *NewImage = NULL;
    int LineNum = 0;

    if ((GifFileOut = EGifOpenFileHandle(fdout)) == NULL) {
	if (EGifCloseFile(GifFileOut) == GIF_ERROR) {
	    PrintGifError();
	}
	exit(EXIT_FAILURE);
    }

    /* OK, interpret directives */
    while (fgets(buf, sizeof(buf), txtin) != (char *)NULL)
    {
	char	*cp;

	++LineNum;

	/*
	 * Skip lines consisting only of whitespace and comments
	 */
#ifndef __COVERITY__
	for (cp = buf; isspace((int)(*cp)); cp++)
	    continue;
	if (*cp == '#' || *cp == '\0')
	    continue;
#endif /* __COVERITY__ */

	/*
	 * If there's a trailing comment, nuke it and all preceding whitespace.
	 * But preserve the EOL.
	 */
	if ((cp = strchr(buf, '#')) && (cp == strrchr(cp, '#')))
	{
	    while (isspace((int)(*--cp)))
		continue;
	    *++cp = '\n';
	    *++cp = '\0';
	}

	/*
	 * Explicit header declarations
	 */

#ifndef __COVERITY__
	// cppcheck-suppress invalidscanf 
	if (sscanf(buf, "screen width %d\n", &GifFileOut->SWidth) == 1)
	    continue;

	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "screen height %d\n", &GifFileOut->SHeight) == 1)
	    continue;

	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "screen colors %d\n", &n) == 1)
	{
	    int	ResBits = BitSize(n);

	    if (n > 256 || n < 0 || n != (1 << ResBits))
	    {
		PARSE_ERROR("Invalid color resolution value.");
		exit(EXIT_FAILURE);
	    }

	    GifFileOut->SColorResolution = ResBits;
	    continue;
	}

	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf,
			"screen background %d\n",
			&GifFileOut->SBackGroundColor) == 1)
	    continue;
#endif /* __COVERITY__ */

	/*
	 * Color table parsing
	 */

	else if (strcmp(buf, "screen map\n") == 0)
	{
	    if (GifFileOut->SColorMap != NULL)
	    {
		PARSE_ERROR("You've already declared a global color map.");
		exit(EXIT_FAILURE);
	    }

	    ColorMapSize = 0;
	    ColorMap = GlobalColorMap;
	    KeyTable = GlobalColorKeys;
	    memset(GlobalColorKeys, '\0', sizeof(GlobalColorKeys));
	}

	else if (strcmp(buf, "image map\n") == 0)
	{
	    if (NewImage == NULL)
	    {
		PARSE_ERROR("No previous image declaration.");
		exit(EXIT_FAILURE);
	    }

	    ColorMapSize = 0;
	    ColorMap = LocalColorMap;
	    KeyTable = LocalColorKeys;
	    memset(LocalColorKeys, '\0', sizeof(LocalColorKeys));
	}

#ifndef __COVERITY__
	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "	rgb %d %d %d is %c",
		   &red, &green, &blue, &KeyTable[ColorMapSize]) == 4)
	{
	    ColorMap[ColorMapSize].Red = red;
	    ColorMap[ColorMapSize].Green = green;
	    ColorMap[ColorMapSize].Blue = blue;
	    ColorMapSize++;
	}
#endif /* __COVERITY__ */

	else if (strcmp(buf, "end\n") == 0)
	{
	    ColorMapObject	*NewMap;


	    NewMap = MakeMapObject(1 << BitSize(ColorMapSize), ColorMap);
	    if (NewMap == (ColorMapObject *)NULL)
	    {
		PARSE_ERROR("Out of memory while allocating new color map.");
		exit(EXIT_FAILURE);
	    }

	    if (NewImage)
		NewImage->ImageDesc.ColorMap = NewMap;
	    else
		GifFileOut->SColorMap = NewMap;
	}

	/* GIF inclusion */
#ifndef __COVERITY__
	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "include %s", InclusionFile) == 1)
	{
	    bool	DoTranslation;
	    GifPixelType	Translation[256];

	    GifFileType	*Inclusion;
	    SavedImage	*NewImage, *CopyFrom;

	    if ((Inclusion = DGifOpenFileName(InclusionFile)) == NULL
		|| DGifSlurp(Inclusion) == GIF_ERROR)
	    {
		PARSE_ERROR("Inclusion read failed.");
		PrintGifError();
		if (Inclusion != NULL) DGifCloseFile(Inclusion);
		if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
		exit(EXIT_FAILURE);
	    }

	    if ((DoTranslation = (GifFileOut->SColorMap!=(ColorMapObject*)NULL)))
	    {
		ColorMapObject	*UnionMap;

		UnionMap = UnionColorMap(GifFileOut->SColorMap,
					 Inclusion->SColorMap, Translation);

		if (UnionMap == NULL)
		{
		    PARSE_ERROR("Inclusion failed --- global map conflict.");
		    PrintGifError();
		    if (Inclusion != NULL) DGifCloseFile(Inclusion);
		    if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
		    exit(EXIT_FAILURE);
		}

		FreeMapObject(GifFileOut->SColorMap);
		GifFileOut->SColorMap = UnionMap;
	    }

	    for (CopyFrom = Inclusion->SavedImages;
		 CopyFrom < Inclusion->SavedImages + Inclusion->ImageCount;
		 CopyFrom++)
	    {
		if ((NewImage = MakeSavedImage(GifFileOut, CopyFrom)) == NULL)
		{
		    PARSE_ERROR("Inclusion failed --- out of memory.");
		    PrintGifError();
		    if (Inclusion != NULL) DGifCloseFile(Inclusion);
		    if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
		    exit(EXIT_FAILURE);
		}
		else if (DoTranslation)
		    ApplyTranslation(NewImage, Translation);

		GifQprintf(
		        "%s: Image %d at (%d, %d) [%dx%d]: from %s\n",
			PROGRAM_NAME, GifFileOut->ImageCount,
			NewImage->ImageDesc.Left, NewImage->ImageDesc.Top,
			NewImage->ImageDesc.Width, NewImage->ImageDesc.Height,
			InclusionFile);
	    }

	    (void) DGifCloseFile(Inclusion);
	}
#endif /* __COVERITY__ */

	/*
	 * Explicit image declarations 
	 */

	else if (strcmp(buf, "image\n") == 0)
	{
	    if ((NewImage = MakeSavedImage(GifFileOut, NULL)) == (SavedImage *)NULL)
	    {
		PARSE_ERROR("Out of memory while allocating image block.");
		exit(EXIT_FAILURE);
	    }

	    /* use global table unless user specifies a local one */
	    ColorMap = GlobalColorMap;
	    KeyTable = GlobalColorKeys;
	}

	/*
	 * Nothing past this point is valid unless we've seen a previous
	 * image declaration.
	 */
	else if (NewImage == (SavedImage *)NULL)
	{
	    (void) fputs(buf, stderr);
	    PARSE_ERROR("Syntax error in header block.");
	    exit(EXIT_FAILURE);
	}

	/*
	 * Accept image attributes
	 */
#ifndef __COVERITY__
	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "image top %d\n", &NewImage->ImageDesc.Top) == 1)
	    continue;

	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf, "image left %d\n", &NewImage->ImageDesc.Left)== 1)
	    continue;
#endif /* __COVERITY__ */

	else if (strcmp(buf, "image interlaced\n") == 0)
	{
	    NewImage->ImageDesc.Interlace = true;
	    continue;
	}

#ifndef __COVERITY__
	// cppcheck-suppress invalidscanf 
	else if (sscanf(buf,
			"image bits %d by %d\n",
			&NewImage->ImageDesc.Width,
			&NewImage->ImageDesc.Height) == 2)
	{
	    int i, j;
	    static GifPixelType *Raster, *cp;
	    int c;

	    if ((Raster = (GifPixelType *) malloc(sizeof(GifPixelType) * NewImage->ImageDesc.Width * NewImage->ImageDesc.Height))
		== NULL) {
		PARSE_ERROR("Failed to allocate raster block, aborted.");
		exit(EXIT_FAILURE);
	    }

	    if (!GifNoisyPrint)
		fprintf(stderr, "%s: Image %d at (%d, %d) [%dx%d]:     ",
		    PROGRAM_NAME, GifFileOut->ImageCount,
		    NewImage->ImageDesc.Left, NewImage->ImageDesc.Top,
		    NewImage->ImageDesc.Width, NewImage->ImageDesc.Height);

	    cp = Raster;
	    for (i = 0; i < NewImage->ImageDesc.Height; i++) {

		char	*dp;

		for (j = 0; j < NewImage->ImageDesc.Width; j++)
		    if ((c = fgetc(txtin)) == EOF) {
			PARSE_ERROR("input file ended prematurely.");
			exit(EXIT_FAILURE);
		    }
		    else if (c == '\n')
		    {
			--j;
			++LineNum;
		    }
		    else if (isspace(c))
			--j;
		    else if ((dp = strchr(KeyTable, c)))
			*cp++ = (dp - KeyTable);
		    else {
			PARSE_ERROR("Invalid pixel value.");
			exit(EXIT_FAILURE);
		    }

		if (!GifNoisyPrint)
		    fprintf(stderr, "\b\b\b\b%-4d", i);
	    }

	    if (!GifNoisyPrint)
		putc('\n', stderr);

	    NewImage->RasterBits = (unsigned char *) Raster;
	}
#endif /* __COVERITY__ */
	else if (sscanf(buf, "comment"))
	{
	    MakeExtension(NewImage, COMMENT_EXT_FUNC_CODE);
	    while (fgets(buf, sizeof(buf), txtin) != (char *)NULL)
		if (strcmp(buf, "end\n") == 0)
		    break;
	        else
		{
		    int Len;

		    buf[strlen(buf) - 1] = '\0';
		    Len = EscapeString(buf, buf);
		    if (AddExtensionBlock(NewImage, Len, (unsigned char *)buf) == GIF_ERROR) {
			PARSE_ERROR("out of memory while adding comment block.");
			exit(EXIT_FAILURE);
		    }
		}
	}
	else if (sscanf(buf, "plaintext"))
	{
	    MakeExtension(NewImage, PLAINTEXT_EXT_FUNC_CODE);
	    while (fgets(buf, sizeof(buf), txtin) != (char *)NULL)
		if (strcmp(buf, "end\n") == 0)
		    break;
	        else
		{
		    int Len;

		    buf[strlen(buf) - 1] = '\0';
		    Len = EscapeString(buf, buf);
		    if (AddExtensionBlock(NewImage, Len, (unsigned char *)buf) == GIF_ERROR) {
			PARSE_ERROR("out of memory while adding plaintext block.");
			exit(EXIT_FAILURE);
		    }
		}
	}
#ifndef __COVERITY__
	else if (sscanf(buf, "extension %02x", &ExtCode))
	{
	    MakeExtension(NewImage, ExtCode);
	    while (fgets(buf, sizeof(buf), txtin) != (char *)NULL)
		if (strcmp(buf, "end\n") == 0)
		    break;
	        else
		{
		    int Len;

		    buf[strlen(buf) - 1] = '\0';
		    Len = EscapeString(buf, buf);
		    if (AddExtensionBlock(NewImage, Len, (unsigned char *)buf) == GIF_ERROR) {
			PARSE_ERROR("out of memory while adding extension block.");
			exit(EXIT_FAILURE);
		    }
		}
	}
#endif /* __COVERITY__ */
	else
	{
	    (void) fputs(buf, stderr);
	    PARSE_ERROR("Syntax error in image description.");
	    exit(EXIT_FAILURE);
	}
    }

    EGifSpew(GifFileOut);
}

static void Gif2Icon(char *FileName,
		     int fdin, int fdout,
		     char NameTable[])
{
    int i, ExtCode, ImageNum = 1;
    GifPixelType *Line, *cp;
    GifRecordType RecordType;
    GifByteType *Extension;
    GifFileType *GifFile;

    if (fdin == -1) {
	if ((GifFile = DGifOpenFileName(FileName)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }
    else {
	/* Use stdin instead: */
	if ((GifFile = DGifOpenFileHandle(fdin)) == NULL) {
	    PrintGifError();
	    exit(EXIT_FAILURE);
	}
    }

    printf("screen width %d\nscreen height %d\n",
	   GifFile->SWidth, GifFile->SHeight);

    printf("screen colors %d\nscreen background %d\n\n",
	   GifFile->SColorResolution,
	   GifFile->SBackGroundColor);

    if (GifFile->SColorMap)
    {
	if (GifFile->SColorMap->ColorCount >= (int)strlen(NameTable))
	{
	    (void) fprintf(stderr,
			   "%s: global color map has unprintable pixels\n",
			   FileName);
	    exit(EXIT_FAILURE);
	}

	printf("screen map\n");

	for (i = 0; i < GifFile->SColorMap->ColorCount; i++)
	    printf("\trgb %03d %03d %03d is %c\n",
		   GifFile->SColorMap ->Colors[i].Red,
		   GifFile->SColorMap ->Colors[i].Green,
		   GifFile->SColorMap ->Colors[i].Blue,
		   NameTable[i]);
	printf("end\n\n");
    }

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
	    printf("image # %d\nimage left %d\nimage top %d\n",
		   ImageNum++,
		   GifFile->Image.Left, GifFile->Image.Top);
	    if (GifFile->Image.Interlace)
		printf("interlaced\n");

	    if (GifFile->Image.ColorMap)
	    {
		if (GifFile->Image.ColorMap->ColorCount >= (int)strlen(NameTable))
		{
		    (void) fprintf(stderr,
				   "%s: global color map has unprintable pixels\n",
				   FileName);
		    exit(EXIT_FAILURE);
		}

		printf("image map\n");

		for (i = 0; i < GifFile->Image.ColorMap->ColorCount; i++)
		    printf("\trgb %03d %03d %03d is %c\n",
			   GifFile->Image.ColorMap ->Colors[i].Red,
			   GifFile->Image.ColorMap ->Colors[i].Green,
			   GifFile->Image.ColorMap ->Colors[i].Blue,
			   NameTable[i]);
		printf("end\n\n");
	    }

	    printf("image bits %d by %d\n",
		   GifFile->Image.Width, GifFile->Image.Height);

	    Line = (GifPixelType *) malloc(GifFile->Image.Width *
					   sizeof(GifPixelType));
	    for (i = 0; i < GifFile->Image.Height; i++) {
		if (DGifGetLine(GifFile, Line, GifFile->Image.Width)
		    == GIF_ERROR) {
		    PrintGifError();
		    exit(EXIT_FAILURE);
		}
		for (cp = Line; cp < Line + GifFile->Image.Width; cp++)
		    putchar(NameTable[*cp]);
		putchar('\n');
	    }
	    free((char *) Line);
	    putchar('\n');

	    break;
	case EXTENSION_RECORD_TYPE:
	    if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
		PrintGifError();
		exit(EXIT_FAILURE);
	    }

	    if (ExtCode == COMMENT_EXT_FUNC_CODE)
		printf("comment\n");
	    else if (ExtCode == PLAINTEXT_EXT_FUNC_CODE)
		printf("plaintext\n");
	    else if (isalpha(ExtCode))
		printf("extension %02x    # %c\n", ExtCode, ExtCode);
	    else
		printf("extension %02x\n", ExtCode);

	    while (Extension != NULL) {
		VisibleDumpBuffer((char *)(Extension + 1), Extension[0]);
		putchar('\n');

		if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
		    PrintGifError();
		    exit(EXIT_FAILURE);
		}
	    }
	    printf("end\n\n");

	    break;
	case TERMINATE_RECORD_TYPE:
	    break;
	default:		/* Should be traps by DGifGetRecordType */
	    break;
	}
    }
    while
	(RecordType != TERMINATE_RECORD_TYPE);

    /* Tell EMACS this is a picture... */
    printf("# The following sets edit modes for GNU EMACS\n");
    printf("# Local ");		/* ...break this up, so that EMACS doesn't */
    printf("Variables:\n");	/* get confused when visiting *this* file! */
    printf("# mode:picture\n");
    printf("# truncate-lines:t\n");
    printf("# End:\n");

    if (fdin == -1)
	(void) printf("# End of %s dump\n", FileName);

    if (DGifCloseFile(GifFile) == GIF_ERROR) {
	PrintGifError();
	exit(EXIT_FAILURE);
    }
}

static int EscapeString(char *cp, char *tp)
/* process standard C-style escape sequences in a string */
{
    char *StartAddr = tp;

    while (*cp)
    {
	int	cval = 0;

	if (*cp == '\\' && strchr("0123456789xX", cp[1]))
	{
	    char *dp, *hex = "00112233445566778899aAbBcCdDeEfF";
	    int dcount = 0;

	    if (*++cp == 'x' || *cp == 'X')
		for (++cp; (dp = strchr(hex, *cp)) && (dcount++ < 2); cp++)
		    cval = (cval * 16) + (dp - hex) / 2;
	    else if (*cp == '0')
		while (strchr("01234567",*cp) != (char*)NULL && (dcount++ < 3))
		    cval = (cval * 8) + (*cp++ - '0');
	    else
		while ((strchr("0123456789",*cp)!=(char*)NULL)&&(dcount++ < 3))
		    cval = (cval * 10) + (*cp++ - '0');
	}
	else if (*cp == '\\')		/* C-style character escapes */
	{
	    switch (*++cp)
	    {
	    case '\\': cval = '\\'; break;
	    case 'n': cval = '\n'; break;
	    case 't': cval = '\t'; break;
	    case 'b': cval = '\b'; break;
	    case 'r': cval = '\r'; break;
	    default: cval = *cp;
	    }
	    cp++;
	}
	else if (*cp == '^')		/* expand control-character syntax */
	{
	    cval = (*++cp & 0x1f);
	    cp++;
	}
	else
	    cval = *cp++;
	*tp++ = cval;
    }

    return(tp - StartAddr);
}

static void VisibleDumpBuffer(char *buf, int len)
/* Visibilize a given string */
{
    char	*cp;

    for (cp = buf; cp < buf + len; cp++)
    {
	if (isprint((int)(*cp)) || *cp == ' ')
	    putchar(*cp);
	else if (*cp == '\n')
	{
	    putchar('\\'); putchar('n');
	}
	else if (*cp == '\r')
	{
	    putchar('\\'); putchar('r');
	}
	else if (*cp == '\b')
	{
	    putchar('\\'); putchar('b');
	}
	else if (*cp < ' ')
	{
	    putchar('\\'); putchar('^'); putchar('@' + *cp);
	}
	else
	    printf("\\0x%02x", *cp);
    }
}

