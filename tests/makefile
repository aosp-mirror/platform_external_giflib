# Regression-test suite for the giflib library and tools

.SUFFIXES: .gif .rgb

# This is what to do by default
test: render-regress \
	gif2raw-regress \
	gifbuild-regress \
	gifclrmp-regress \
	giffilter-regress \
	gifinter-regress \
	gifsponge-regress \
	giftext-regress \
	giftool-regress \
	misc-regress
	@echo "No output is good news"

rebuild: render-buildregress misc-buildregress \
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
	    if echo "Testing RGB rendering of $${test}" >&2; \
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

gif2raw-regress:
	@echo "gif2raw: Checking that it can self-invert."
	@$(UTILS)/gif2raw <$(PICS)/treescap.gif | $(UTILS)/gif2raw -s 40 40 -p treescap.map | $(UTILS)/gif2rgb | cmp - treescap.rgb

gifbuild-regress:
	@echo "gifbuild: Checking idempotency on an icon file."
	@$(UTILS)/gifbuild <$(PICS)/sample.ico | $(UTILS)/gifbuild -d >/tmp/sample-1.ico; $(UTILS)/gifbuild </tmp/sample-1.ico | $(UTILS)/gifbuild -d >/tmp/sample-2.ico; diff -u /tmp/sample-1.ico /tmp/sample-2.ico; rm /tmp/sample-*.ico
	@echo "gifbuild: Checking idempotency on an animation."
	@$(UTILS)/gifbuild -d <$(PICS)/fire.gif >/tmp/fire1.ico
	@$(UTILS)/gifbuild </tmp/fire1.ico >/tmp/fire2.gif
	@$(UTILS)/gifbuild -d </tmp/fire2.gif >/tmp/fire2.ico
	@diff -u /tmp/fire1.ico /tmp/fire2.ico

gifclrmp-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "gifclrmap: Checking colormap of $${test}" >&2; \
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

giffilter-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "giffiltr: Testing copy of $${test}" >&2; \
	    $(UTILS)/giffilter <$${test} | $(UTILS)/gif2rgb >/tmp/regress 2>&1; \
	    then cmp $${stem}.rgb /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress

gifinter-regress:
	@echo "gifinter: Checking that it de-interlaces correctly."
	@$(UTILS)/gifinter -s <$(PICS)/treescap-interlaced.gif | $(UTILS)/gif2rgb | cmp - treescap.rgb
	@echo "gifinter: Checking that it interlaces correctly."
	@$(UTILS)/gifinter -i <$(PICS)/treescap.gif | $(UTILS)/gif2rgb | cmp - treescap-interlaced.rgb

gifinto-regress:
	@echo "gifinto: Checking behavior on short files."
	@rm -f /tmp/giflib.tmp
	@$(UTILS)/gifinto <$(PICS)/porsche.gif /tmp/giflib.tmp
	@if test ! -f /tmp/giflib.tmp; then echo "gifinto failed to create a file when it should have."; fi
	@rm -f /tmp/giflib.tmp
	@echo "0123456789" | $(UTILS)/gifinto /tmp/giflib.tmp 2>/dev/null
	@if test -f /tmp/giflib.tmp; then echo "gifinto created a file when it shouldn't have."; fi
	@rm -f /tmp/giflib.tmp

gifsponge-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "gifsponge: Testing copy of $${test}" >&2; \
	    $(UTILS)/gifsponge <$${test} | $(UTILS)/gif2rgb >/tmp/regress 2>&1; \
	    then cmp $${stem}.rgb /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress

giftext-regress:
	@for test in $(GIFS); \
	do \
	    stem=`basename $${test} | sed -e "s/.gif$$//"`; \
	    if echo "giftext: Checking text dump of $${test}" >&2; \
	    $(UTILS)/giftext <$${test} >/tmp/regress 2>&1; \
	    then diff -u $${stem}.dmp /tmp/regress; \
	    else echo "*** Nonzero return status on $${test}!"; exit 1; fi; \
	done
	@rm -f /tmp/regress
giftext-buildregress:
	@for test in $(GIFS); do \
		stem=`basename $${test} | sed -e "s/.gif$$//"`; \
		echo "Remaking $${stem}.dmp"; \
		$(UTILS)/giftext <$${test} >$${stem}.dmp; \
	done

giftool-regress:
	@echo "giftool: Checking that expensive copy via giftool is faithful."
	@$(UTILS)/giftool <$(PICS)/gifgrid.gif | $(UTILS)/gif2rgb | cmp - gifgrid.rgb
	@echo "giftool: Checking that it de-interlaces correctly."
	@$(UTILS)/giftool -i on <$(PICS)/treescap-interlaced.gif | $(UTILS)/gif2rgb | cmp - treescap.rgb
	@echo "giftool: Checking that it interlaces correctly."
	@$(UTILS)/giftool -i off <$(PICS)/treescap.gif | $(UTILS)/gif2rgb | cmp - treescap-interlaced.rgb

misc-buildregress:
	$(UTILS)/gifwedge >wedge.gif
	$(UTILS)/gifpos -s 680 480 <$(PICS)/treescap.gif | $(UTILS)/gifbuild -d >treescap.ico

misc-regress:
	@echo "gifwedge: Checking wedge generation."
	@$(UTILS)/gifwedge | cmp - wedge.gif
	@echo "gifpos: Checking behavior of gifpos."
	@$(UTILS)/gifpos -s 680 480 <$(PICS)/treescap.gif | $(UTILS)/gifbuild -d | diff -u treescap.ico -
