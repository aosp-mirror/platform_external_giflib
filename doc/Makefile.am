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
TXTSOURCES = index.html intro.xml gif_lib.xml liberror.xml
STDSOURCES = gif89.txt lzgif.txt
SOURCES = $(MANSOURCES) $(TXTSOURCES) $(STDSOURCES)

allhtml: $(SOURCES:.xml=.html)

manpages: $(MANSOURCES:.xml=.1)

# Control what goes in the distribution tarball.
# We include all of the XML, the generated HTML, and generated manual pages, 
# so people working from the distribution tarball won't need xmlto. 
UTILHELP = $(MANSOURCES:.xml=.html)
LIBRARYHELP = GifFileType.png \
	     $(SOURCES) $(TXTSOURCES:.xml=.html) $(MANSOURCES:.xml=.1)
EXTRA_DIST = $(UTILHELP) $(LIBRARYHELP)

# Prepare the website directory to deliver an update
website: allhtml
	rm -fr staging; mkdir staging; cp $(SOURCES:.xml=.html) staging

clean-local:
	rm -fr staging $(SOURCES:.xml=.html) $(MANSOURCES:.xml=.1)
