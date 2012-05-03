SUFFIXES = .xml .html .txt .1

.xml.html:
	xmlto xhtml-nochunks $<

.xml.1:
	xmlto man $<

.xml.txt:
	xmlto txt $<

all: allhtml manpages

MANSOURCES = \
	gif2rgb.xml gif2x11.xml gifasm.xml gifbg.xml gifclip.xml gifclrmp.xml \
	gifcolor.xml gifcomb.xml gifcompose.xml giffiltr.xml giffix.xml \
	gifflip.xml gifhisto.xml gifinter.xml gifinto.xml gifovly.xml \
	gifpos.xml gifrotat.xml gifrsize.xml gifspnge.xml giftext.xml \
	gifwedge.xml icon2gif.xml raw2gif.xml rgb2gif.xml text2gif.xml
TXTSOURCES = index.xml gif_lib.xml liberror.xml
SOURCES = $(MANSOURCES) $(TXTSOURCES)

allhtml: $(SOURCES:.xml=.html)

manpages: $(MANSOURCES:.xml=.1)

# Control what goes in the distribution tarball.
# We include all of the XML, the generated HTML, and generated manual pages, 
# so people working from the distribution tarball won't need xmlto. 
UTILHELP = $(MANSOURCES:.xml=.html)
LIBRARYHELP = GifFileType.png \
	     gif89.txt \
	     lzgif.txt \
	     $(SOURCES) $(TXTSOURCES:.xml=.html) $(MANSOURCES:.xml=.1)
EXTRA_DIST = $(UTILHELP) $(LIBRARYHELP)

distclean-local:
	rm -f $(SOURCES:.xml=.html) $(MANSOURCES:.xml=.1)
