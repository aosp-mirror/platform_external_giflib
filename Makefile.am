SUBDIRS = lib util doc pic
WINDOWS = $(top_srcdir)/windows/GifWin.cpp \
	  $(top_srcdir)/windows/GifWin.h \
	  $(top_srcdir)/windows/StdAfx.h \
	  $(top_srcdir)/windows/gifshow.cpp \
	  $(top_srcdir)/windows/gifshow.dsp \
	  $(top_srcdir)/windows/gifshow.dsw \
	  $(top_srcdir)/windows/gifshow.h \
	  $(top_srcdir)/windows/gifshow.ico \
	  $(top_srcdir)/windows/gifshow.rc \
	  $(top_srcdir)/windows/libungif.dsp \
	  $(top_srcdir)/windows/resource.h

GAMEBOYADVANCE = $(top_srcdir)/gba/giftest.c \
		 $(top_srcdir)/gba/giftest.dsp \
		 $(top_srcdir)/gba/giftest.mak \
		 $(top_srcdir)/gba/res/cover.gif \
		 $(top_srcdir)/gba/res/porsche-240x160.gif \
		 $(top_srcdir)/gba/res/x-trans.gif

EXTRA_DIST = Makefile.unx \
	     ONEWS \
	     autogen.sh \
	     test-unx \
	     TODO \
	     BUGS \
	     DEVELOPERS \
	     $(WINDOWS) \
	     $(GAMEBOYADVANCE)
#TESTS=test-unx

# Auditing tools.  Presently run only on the library, not the utilities.
# These don't run clean yet.

SUPPRESSIONS = -boolops -nullret
splint: 
	splint +quiet +posixlib $(SUPPRESSIONS) lib/*.[ch]


cppcheck:
	@echo 'Expect 1 error, "Resource leak: FileHandle"'.
	cppcheck --template gcc --all --force lib/*.[ch]

