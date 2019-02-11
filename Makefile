# Top-level Unix makefile for the GIFLIB package
# Should work for all Unix versions
#
# If your platform has the OpenBSD reallocarray(3) call, you may
# add -DHAVE_REALLOCARRAY to CFLAGS to use that, saving a bit
# of code space in the shared library.

#
CC    = gcc
OFLAGS = -O0 -g
#OFLAGS  = -O2 -fwhole-program
CFLAGS  = -std=gnu99 -fPIC -Wall -Wno-format-truncation $(OFLAGS)
LDFLAGS = -g

SHELL = /bin/sh
TAR = tar
INSTALL = install

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib

# No user-serviceable parts below this line

VERSION=5.1.4
LIBMAJOR=7
LIBMINOR=1
LIBPOINT=0

SOURCES = dgif_lib.c egif_lib.c getarg.c gifalloc.c gif_err.c gif_font.c \
	gif_hash.c openbsd-reallocarray.c qprintf.c quantize.c
HEADERS = getarg.h  gif_hash.h  gif_lib.h  gif_lib_private.h
OBJECTS = $(SOURCES:.c=.o)

# Some utilities are installed
INSTALLABLE = \
	gif2rgb \
	gifbuild \
	gifecho \
	giffilter \
	giffix \
	gifinto \
	giftext \
	giftool \
	gifsponge \
	gifclrmp

# Some utilities are only used internally for testing.
# There is a parallel list in doc/Makefile.
# These are all candidates for removal in future releases.
UTILS = $(INSTALLABLE) \
	gifbg \
	gifcolor \
	gifhisto \
	gifsponge \
	gifwedge

LDLIBS=giflib.a -lm

all: giflib.so giflib.a $(UTILS)
	cd doc; make

$(UTILS):: giflib.a

giflib.so: $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) -shared $(OFLAGS) -o giflib.so $(OBJECTS)
	ln -sf giflib.so giflib.so.$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)
	ln -sf giflib.so giflib.so.$(LIBMAJOR)

giflib.a: $(OBJECTS) $(HEADERS)
	ar rcs giflib.a $(OBJECTS)

clean:
	rm -f $(UTILS) $(TARGET) libgetarg.a giflib.a giflib.so
	rm -f giflib.so.$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)
	rm -f giflib.so.$(LIBMAJOR)
	rm -fr doc/*.1 *.html doc/staging

make check: all
	cd tests; make

# Installation/uninstallation

install: all install-bin install-man
install-bin: $(INSTALLABLE)
	$(INSTALL) -d "$(PREFIX)/bin"
	$(INSTALL) $^ "$(PREFIX)/bin"
install-man:
	$(INSTALL) -d "$(PREFIX)/share/man/man1"
	$(INSTALL) -m 644 doc/*.1 "$(PREFIX)/share/man/man1"
uninstall: uninstall-man uninstall-bin
uninstall-bin:
	cd "$(PREFIX)/bin"; rm $(INSTALLABLE)
uninstall-man:
	cd $(target)/share/man/man1/ && rm -f $(shell cd doc >/dev/null ; echo *.1)

# Make distribution tarball
#
# We include all of the XML, and also generated manual pages
# so people working from the distribution tarball won't need xmlto.

EXTRAS = Makefile \
	     tests \
	     README \
	     NEWS \
	     TODO \
	     COPYING \
	     ChangeLog \
	     build.adoc \
	     history.adoc \
	     control \
	     doc/whatsinagif \

DSOURCES = Makefile *.[ch]
DOCS = doc/*.1 doc/*.xml doc/*.txt doc/index.html.in doc/00README
ALL =  $(DSOURCES) $(DOCS) tests pic $(EXTRAS)
giflib-$(VERSION).tar.gz: $(ALL)
	$(TAR) --transform='s:^:giflib-$(VERSION)/:' --show-transformed-names -cvzf giflib-$(VERSION).tar.gz $(ALL)

dist: giflib-$(VERSION).tar.gz

# Auditing tools.

# cppcheck should run clean
cppcheck:
	cppcheck --inline-suppr --template gcc --enable=all --suppress=unusedFunction --force *.[ch]

# release using the shipper tool
release: all
	cd doc; make website
	shipper --no-stale version=$(VERSION) | sh -e -x
	rm -fr doc/staging

# Refresh the website
refresh: all
	cd doc; make website
	shipper --no-stale -w version=$(VERSION) | sh -e -x
	rm -fr doc/staging
