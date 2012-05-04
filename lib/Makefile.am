## Process this file with automake to produce a Makefile.in for giflib.

EXTRA_DIST = Makefile.unx

lib_LTLIBRARIES = libgif.la

include_HEADERS = gif_lib.h

libgif_la_SOURCES = dgif_lib.c  \
                    egif_lib.c  \
                    gif_font.c  \
					gif_hash.c  \
					gif_hash.h	\
                    gifalloc.c  \
                    gif_err.c   \
					gif_lib_private.h

libgif_la_LDFLAGS = -version-info @SHLIBVERSION@
libgif_la_LIBADD = @DEVS@
libgif_CFLAGS = $(X_CFLAGS) $(AM_CFLAGS)
