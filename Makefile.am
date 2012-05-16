SUBDIRS = lib util doc pic

EXTRA_DIST = Makefile.unx \
	     autogen.sh \
	     tests \
	     TODO \
	     BUGS \
	     build.txt \
	     api.txt \
	     history.txt \
	     control \
	     $(WINDOWS)

# Auditing tools.

# Coverity - avoid making doc as xmlto craps out under cov-build
coverity:
	cd lib; make
	cd util; make

# cppcheck should run clean
cppcheck:
	cppcheck -Ilib --inline-suppr --template gcc --enable=all --suppress=unusedStructMember --suppress=unusedFunction --force lib/*.[ch] util/*.[ch]

# splint is nowhere near running clean
SPLINT_SUPPRESSIONS = -boolops -nullret
splint: 
	splint +quiet +posixlib $(SPLINT_SUPPRESSIONS) lib/*.[ch]

# release using the shipper tool
release: dist
	cd doc; make website
	shipper -u -m -t; make clean
	rm -fr doc/staging
