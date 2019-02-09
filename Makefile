# Top-level Unix makefile for the GIFLIB package
# Should work for all Unix versions
#
CC    = gcc
CFLAGS  = -std=gnu99 -fPIC -pedantic -Wall -Wextra
DFLAGS  = -O0
OFLAGS  = -O2 -fwhole-program

SHELL = /bin/sh
TAR = tar
INSTALL = install

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib

# No user-serviceable parts below this line

VERSION=5.1.4
LIBMAJOR=7
LIBMINOR=0
LIBPOINT=0

SOURCES = $(shell echo lib/*.c)
HEADERS = $(shell echo lib/*.h)
OBJECTS = $(SOURCES:.c=.o)

# Some utilitoes are installed
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

all: giflib.so $(UTILS)
	cd doc; make

giflib.so: $(OBJECTS)
	$(CC) $(CFLAGS) -shared $(OFLAGS) -o giflib.so $(OBJECTS)
	ln giflib.so giflib.so.$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)
	ln giflib.so giflib.so.$(LIBMAJOR)

giflib.a: $(OBJECTS)
	ar rcs giflib.a $(OBJECTS)

GETARG=util/getarg.c util/getarg.h util/qprintf.c 
gifbg: util/gifbg.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifbg util/gifbg.o $(GETARG:.c=.o) giflib.a
gifcolor: util/gifcolor.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifcolor util/gifcolor.o $(GETARG:.c=.o) giflib.a
gifhisto: util/gifhisto.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifhisto util/gifhisto.o $(GETARG:.c=.o) giflib.a
gif2rgb: util/gif2rgb.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gif2rgb util/gif2rgb.o $(GETARG:.c=.o) giflib.a
gifbuild: util/gifbuild.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifbuild util/gifbuild.o $(GETARG:.c=.o) giflib.a
gifecho: util/gifecho.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifecho util/gifecho.o $(GETARG:.c=.o) giflib.a
giffix: util/giffix.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o giffix util/giffix.o $(GETARG:.c=.o) giflib.a
giffilter: util/giffilter.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o giffilter util/giffilter.o $(GETARG:.c=.o) giflib.a
gifinto: util/gifinto.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifinto util/gifinto.o $(GETARG:.c=.o) giflib.a
gifsponge: util/gifsponge.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifsponge util/gifsponge.o $(GETARG:.c=.o) giflib.a
giftext: util/giftext.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o giftext util/giftext.o $(GETARG:.c=.o) giflib.a
giftool: util/giftool.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o giftool util/giftool.o $(GETARG:.c=.o) giflib.a
gifwedge: util/gifwedge.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifwedge util/gifwedge.o $(GETARG:.c=.o) giflib.a
gifclrmp: util/gifclrmp.o $(GETARG:.c=.o) giflib.a
	$(CC) $(CFLAGS) $(OFLAGS) -o gifclrmp util/gifclrmp.o -lm $(GETARG:.c=.o) giflib.a

clean:
	rm -f $(UTILS) $(TARGET) libgetarg.a giflib.a giflib.so
	rm -f giflib.so.$(LIBMAJOR).$(LIBMINOR).$(LIBPOINT)
	rm -f giflib.so.$(LIBMAJOR)
	rm -fr doc/*.1 *.html doc/staging

check: all
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

DSOURCES = Makefile lib/*.[ch] util/*.[ch]
DOCS = doc/*.1 doc/*.xml doc/*.txt doc/index.html.in doc/00README
ALL =  $(DSOURCES) $(DOCS) tests pic $(EXTRAS)
giflib-$(VERSION).tar.gz: $(ALL)
	$(TAR) --transform='s:^:giflib-$(VERSION)/:' --show-transformed-names -cvzf giflib-$(VERSION).tar.gz $(ALL)

dist: giflib-$(VERSION).tar.gz

# Auditing tools.

# cppcheck should run clean
cppcheck:
	cppcheck -Ilib --inline-suppr --template gcc --enable=all --suppress=unusedStructMember --suppress=unusedFunction --force lib/*.[ch] util/*.[ch]

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
