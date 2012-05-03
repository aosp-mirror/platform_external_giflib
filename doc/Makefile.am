UTILHELP = gif2rgb.html \
	   gif2x11.html \
	   gifasm.html \
	   gifbg.html \
	   gifclip.html \
	   gifclrmp.html \
	   gifcolor.html \
	   gifcomb.html \
	   gifcompose.html \
	   giffiltr.html \
	   giffix.html \
	   gifflip.html \
	   gifhisto.html \
	   gifinter.html \
	   gifinto.html \
	   gifovly.html \
	   gifpos.html \
	   gifrotat.html \
	   gifrsize.html \
	   gifspnge.html \
	   giftext.html \
	   gifwedge.html \
	   icon2gif.html \
	   raw2gif.html \
	   rgb2gif.html \
	   text2gif.html
LIBRARYHELP = GifFileType.png \
	     gif89.txt \
	     gif_lib.html \
	     index.html \
	     liberror.html \
	     lzgif.txt
EXTRA_DIST = $(UTILHELP) $(LIBRARYHELP)

SUFFIXES = .xml .html .txt

.xml.html:
	xmlto xhtml-nochunks $<

.xml.txt:
	xmlto txt $<

all: allhtml alltxt alldvi

SOURCES = index.xml gif2rgb.xml gif2x11.xml gifasm.xml gifbg.xml

alltxt: $(SOURCES:.xml=.txt)

allhtml: $(SOURCES:.xml=.html)
