SUFFIXES = .xml .html .txt .asc .1

.xml.html:
	xmlto xhtml-nochunks $<

.xml.1:
	xmlto man $<

.xml.txt:
	xmlto txt $<

.asc.html:
	asciidoc $<

all: allhtml manpages

XMLMAN = gif2rgb.xml gif2x11.xml gifasm.xml gifbg.xml gifclip.xml	\
	gifclrmp.xml gifcolor.xml gifcomb.xml giffilter.xml giffix.xml	\
	gifflip.xml gifhisto.xml gifinter.xml gifinto.xml gifovly.xml	\
	gifpos.xml gifrotate.xml gifresize.xml gifsponge.xml giftext.xml	\
	giftool.xml gifwedge.xml gifbuild.xml raw2gif.xml gifecho.xml
XMLDOC = intro.xml gif_lib.xml
XMLALL = $(XMLMAN) $(XMLDOC)
TXTDOC = 00README gif87.txt gif89.txt lzgif.txt
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
	cp -r $(XMLALL:.xml=.html) $(SITEHTML) *.txt whatsinagif staging
	../util/gifresize -S 50 50 <../pic/gifgrid.gif >staging/giflib-logo.gif
	asciidoc - <../history.asc >staging/history.html

clean-local:
	rm -fr staging $(XMLALL:.xml=.html) $(XMLMAN:.xml=.1)
