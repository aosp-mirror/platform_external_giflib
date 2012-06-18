
noinst_LIBRARIES = libgetarg.a
bin_PROGRAMS = gif2rgb gifbg giffix gifhisto gifinter gifinto \
    giftext gifwedge gif2raw gifecho gifsponge giffilter \
    gifbuild gifcolor giftool @COMPILABLE_EXTRAS@
EXTRA_PROGRAMS = gifclrmp

EXTRA_DIST=Makefile.unx $(bin_SCRIPTS)

LDADD = libgetarg.a ../lib/libgif.la

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/lib

libgetarg_a_SOURCES = getarg.c getarg.h \
                    qprintf.c

gifclrmp_SOURCES = gifclrmp.c
gifclrmp_LDADD = $(LDADD) @MATH_LIB@

gif2rgb_SOURCES = gif2rgb.c
gifbg_SOURCES = gifbg.c
giffix_SOURCES = giffix.c
gifhisto_SOURCES = gifhisto.c
gifinter_SOURCES = gifinter.c
gifinto_SOURCES = gifinto.c
giftext_SOURCES = giftext.c
gifwedge_SOURCES = gifwedge.c
gif2raw_SOURCES = gif2raw.c
gifecho_SOURCES = gifecho.c
gifsponge_SOURCES = gifsponge.c
giffilter_SOURCES = giffilter.c
gifbuild_SOURCES = gifbuild.c
gifcolor_SOURCES = gifcolor.c
giftool_SOURCES = giftool.c
