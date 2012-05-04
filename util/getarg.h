/***************************************************************************
 * Support routines for the giflib utilities
 *
 ***************************************************************************
 * History:
 * 11 Mar 88 - Version 1.0 by Gershon Elber.
 **************************************************************************/

#ifndef _GETARG_H
#define _GETARG_H

#include <stdbool.h>

#define VERSION_COOKIE " Version %d.%d, "

/***************************************************************************
 * Error numbers as returned by GAGetArg routine:
 **************************************************************************/ 
#define CMD_ERR_NotAnOpt  1    /* None Option found. */
#define CMD_ERR_NoSuchOpt 2    /* Undefined Option Found. */
#define CMD_ERR_WildEmpty 3    /* Empty input for !*? seq. */
#define CMD_ERR_NumRead   4    /* Failed on reading number. */
#define CMD_ERR_AllSatis  5    /* Fail to satisfy (must-'!') option. */

bool GAGetArgs(int argc, char **argv, char *CtrlStr, ...);
void GAPrintErrMsg(int Error);
void GAPrintHowTo(char *CtrlStr);

/******************************************************************************
 * O.K., here are the routines from QPRINTF.C.              
******************************************************************************/
extern bool GifNoisyPrint;
extern void GifQprintf(char *Format, ...);

/******************************************************************************
 * O.K., here are the routines from GIF_ERR.C.              
******************************************************************************/
extern void PrintGifError(void);
extern int GifLastError(void);

/* These used to live in the library header */
#define GIF_MESSAGE(Msg) fprintf(stderr, "\n%s: %s\n", PROGRAM_NAME, Msg)
#define GIF_EXIT(Msg)    { GIF_MESSAGE(Msg); exit(-3); }

#endif /* _GETARG_H */
