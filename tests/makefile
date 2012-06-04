# Regression-test suite for the giflib library and tools

.SUFFIXES: .gif .rgb

# This is what to do by default
test: render-regress util-regress

UTILS = ../util
PICS = ../pic

GIFS := $(shell ls ../pic/*.gif)

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
	@echo "Checking wedge generation."
	@$(UTILS)/gifwedge | cmp - wedge.gif
	@echo "Checking that expensive copy via giftool is faithful."
	@$(UTILS)/giftool <$(PICS)/cover.gif | $(UTILS)/gif2rgb | cmp - cover.rgb
	@echo "Checking that giftool de-interlaces correctly."
	@$(UTILS)/giftool -i <$(PICS)/treescap-interlaced.gif | $(UTILS)/gif2rgb | cmp - treescap.rgb
	@echo "Checking that giftool interlaces correctly."
	@$(UTILS)/giftool -I <$(PICS)/treescap.gif | $(UTILS)/gif2rgb | cmp - treescap-interlaced.rgb
	@echo "No output is good news"
