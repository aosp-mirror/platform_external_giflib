# Regression-test suite for the giflib library and tools

.SUFFIXES: .gif .rgb

# This is what to do by default
test: render-regress util-regress gifspnge-regress \
		gifclrmp-regress giftext-regress
	@echo "No output is good news"

rebuild: render-buildregress util-buildregress \
		gifclrmp-buildregress giftext-buildregress

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

gifclrmp-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "Checking colormap of $${test}" >&2; \
	    $(UTILS)/gifclrmp <$${test} >/tmp/regress 2>&1; \
	    then cmp $${stem}.map /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress
gifclrmp-buildregress:
	@for test in $(GIFS); do \
		stem=`basename $${test} | sed -e "s/.gif$$//"`; \
		echo "Remaking $${stem}.map"; \
		$(UTILS)/gifclrmp <$${test} >$${stem}.map; \
	done

giftext-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "Checking text dump of $${test}" >&2; \
	    $(UTILS)/giftext <$${test} >/tmp/regress 2>&1; \
	    then cmp $${stem}.dmp /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress
giftext-buildregress:
	@for test in $(GIFS); do \
		stem=`basename $${test} | sed -e "s/.gif$$//"`; \
		echo "Remaking $${stem}.dmp"; \
		$(UTILS)/giftext <$${test} >$${stem}.dmp; \
	done

gifspnge-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "Testing gifspnge copy of $${test}" >&2; \
	    $(UTILS)/gifspnge <$${test} | $(UTILS)/gif2rgb >/tmp/regress 2>&1; \
	    then cmp $${stem}.rgb /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress

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
	@echo "Checking idempotency of icon2gif"
	@$(UTILS)/icon2gif <$(PICS)/sample.ico | $(UTILS)/icon2gif -d >/tmp/sample-1.ico; $(UTILS)/icon2gif </tmp/sample-1.ico | $(UTILS)/icon2gif -d >/tmp/sample-2.ico; diff -u /tmp/sample-1.ico /tmp/sample-2.ico; rm /tmp/sample-*.ico
	@echo "Checking behavior of gifinto"
	@rm -f /tmp/giflib.tmp
	@$(UTILS)/gifinto <$(PICS)/porsche.gif /tmp/giflib.tmp
	@if test ! -f /tmp/giflib.tmp; then echo "gifinto failed to create a file when it should have."; fi
	@rm -f /tmp/giflib.tmp
	@echo "0123456789" | $(UTILS)/gifinto /tmp/giflib.tmp
	@if test -f /tmp/giflib.tmp; then echo "gifinto created a file when it shouldn't have."; fi
	@rm -f /tmp/giflib.tmp
