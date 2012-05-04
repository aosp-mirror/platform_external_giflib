# Regression-test suite for the giflib library and tools

.SUFFIXES: .gif .rgb

# This is what to do by default
test: render-regress util-regress

GIFS := $(shell ls ../pic/*.gif)

UTILS = ../util

# Test decompression and rendering by unpacking images,
# converting them to RGB, and comparing that result to a check file.
render-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "Testing $${test}" >&2; \
	    $(UTILS)/gif2rgb -1 -o /tmp/regress $${test} 2>&1; \
	    then cmp $${stem}.rgb /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress
render-buildregress:
	@for test in $(GIFS); do \
		stem=`basename $${test} | sed -e "s/.gif$$//"`; \
		echo "Remaking $${stem}.rgb"; \
		$(UTILS)/gif2rgb -1 -o $${stem}.rgb $${test}; \
	done

util-buildregress:
	$(UTILS)/gifwedge >wedge.gif

util-regress:
	@echo "No output is good news"
	@$(UTILS)/gifwedge | cmp - wedge.gif
