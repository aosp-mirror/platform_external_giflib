SUFFIXES = .xml .html .txt .1

.xml.html:
	xmlto xhtml-nochunks $<

.xml.1:
	xmlto man $<

.xml.txt:
	xmlto txt $<

all: allhtml manpages

XMLMAN = \
	gif2rgb.xml gif2x11.xml gifasm.xml gifbg.xml gifclip.xml gifclrmp.xml \
	gifcolor.xml gifcomb.xml gifcompose.xml giffiltr.xml giffix.xml \
	gifflip.xml gifhisto.xml gifinter.xml gifinto.xml gifovly.xml \
	gifpos.xml gifrotat.xml gifrsize.xml gifspnge.xml giftext.xml \
	gifwedge.xml icon2gif.xml raw2gif.xml rgb2gif.xml text2gif.xml
XMLDOC = intro.xml gif_lib.xml liberror.xml
XMLALL = $(XMLMAN) $(XMLDOC)
TXTDOC = gif89.txt lzgif.txt
SOURCES = $(XMLALL) $(TXTDOC)
SITEHTML = index.html

allhtml: $(XMLALL:.xml=.html)

manpages: $(XMLMAN:.xml=.1)

# Control what goes in the distribution tarball.
# We include all of the XML, and also generated manual pages 
# so people working from the distribution tarball won't need xmlto. 
EXTRA_DIST = $(SOURCES) $(XMLMAN:.xml=.1)

# Prepare the website directory to deliver an update
website: allhtml
	rm -fr staging; mkdir staging; 
	cp $(XMLALL:.xml=.html) $(SITEHTML) GifFileType.png staging

clean-local:
	rm -fr staging $(XMLALL:.xml=.html) $(XMLMAN:.xml=.1)
