SUBDIRS = lib util doc pic

EXTRA_DIST = Makefile.unx \
	     ONEWS \
	     autogen.sh \
	     tests \
	     TODO \
	     BUGS \
	     build.txt \
	     api.txt \
	     history.txt \
	     $(WINDOWS)

# Auditing tools.

# cppcheck should run clean
cppcheck:
	cppcheck -Ilib --inline-suppr --template gcc --enable=all --suppress=unusedStructMember --suppress=unusedFunction --force lib/*.[ch] util/*.[ch]

# splint is nowhere near running clean
SPLINT_SUPPRESSIONS = -boolops -nullret
splint: 
	splint +quiet +posixlib $(SPLINT_SUPPRESSIONS) lib/*.[ch]

