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

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
MANDIR = $(PREFIX)/share/man

# No user-serviceable parts below this line

VERSION=5.1.5
LIBMAJOR=7
LIBMINOR=1
LIBPOINT=0
LIBVER=$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)

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

LDLIBS=libgif.a -lm

all: libgif.so libgif.a $(UTILS)
	$(MAKE) -C doc

$(UTILS):: libgif.a

libgif.so: $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) -shared $(OFLAGS) -o libgif.so $(OBJECTS)

libgif.a: $(OBJECTS) $(HEADERS)
	ar rcs libgif.a $(OBJECTS)

clean:
	rm -f $(UTILS) $(TARGET) libgetarg.a libgif.a libgif.so
	rm -f libgif.so.$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)
	rm -f libgif.so.$(LIBMAJOR)
	rm -fr doc/*.1 *.html doc/staging

check: all
	$(MAKE) -C tests

# Installation/uninstallation

install: all install-bin install-include install-lib install-man
install-bin: $(INSTALLABLE)
	$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	$(INSTALL) $^ "$(DESTDIR)$(BINDIR)"
install-include:
	$(INSTALL) -d "$(DESTDIR)$(INCDIR)"
	$(INSTALL) -m 644 gif_lib.h "$(DESTDIR)$(INCDIR)"
install-lib:
	$(INSTALL) -d "$(DESTDIR)$(LIBDIR)"
	$(INSTALL) -m 644 libgif.a "$(DESTDIR)$(LIBDIR)/libgif.a"
	$(INSTALL) -m 755 libgif.so "$(DESTDIR)$(LIBDIR)/libgif.so.$(LIBVER)"
	ln -sf libgif.so.$(LIBVER) "$(DESTDIR)$(LIBDIR)/libgif.so.$(LIBMAJOR)"
	ln -sf libgif.so.$(LIBMAJOR) "$(DESTDIR)$(LIBDIR)/libgif.so"
install-man:
	$(INSTALL) -d "$(DESTDIR)$(MANDIR)"
	$(INSTALL) -m 644 doc/*.1 "$(DESTDIR)$(MANDIR)"
uninstall: uninstall-man uninstall-include uninstall-lib uninstall-bin
uninstall-bin:
	cd "$(DESTDIR)$(BINDIR)" && rm -f $(INSTALLABLE)
uninstall-include:
	rm -f "$(DESTDIR)$(INCDIR)/gif_lib.h"
uninstall-lib:
	cd "$(DESTDIR)$(LIBDIR)" && \
		rm -f libgif.a libgif.so libgif.so.$(LIBMAJOR) libgif.so.$(LIBVER)
uninstall-man:
	cd "$(DESTDIR)$(MANDIR)" && rm -f $(shell cd doc >/dev/null && echo *.1)

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
DOCS = doc/*.1 doc/*.xml doc/*.txt doc/index.html.in doc/00README doc/Makefile
ALL =  $(DSOURCES) $(DOCS) tests pic $(EXTRAS)
giflib-$(VERSION).tar.gz: $(ALL)
	$(TAR) --transform='s:^:giflib-$(VERSION)/:' --show-transformed-names -cvzf giflib-$(VERSION).tar.gz $(ALL)

dist: giflib-$(VERSION).tar.gz

# Auditing tools.

# cppcheck should run clean
cppcheck:
	cppcheck --inline-suppr --template gcc --enable=all --suppress=unusedFunction --force *.[ch]

# Verify the build
distcheck: all
	$(MAKE) giflib-$(VERSION).tar.gz
	tar xzvf giflib-$(VERSION).tar.gz
	$(MAKE) -C giflib-$(VERSION)
	rm -fr giflib-$(VERSION)

# release using the shipper tool
release: all distcheck
	$(MAKE) -C doc website
	shipper --no-stale version=$(VERSION) | sh -e -x
	rm -fr doc/staging

# Refresh the website
refresh: all
	$(MAKE) -C doc website
	shipper --no-stale -w version=$(VERSION) | sh -e -x
	rm -fr doc/staging
