# Create ACM-acceptable PDF document

#AREADER = /opt/areader5/bin/acroread

TARGET=reportdpg
SOURCES=reportdpg.tex 

BACKUPPATH = ./backup
PAPER = reportdpg

# make pdf by default
all: ${TARGET}.pdf

# it doesn't really need the .dvi, but this way all the refs are right
# ${TARGET}.pdf : ${TARGET}.ps
# 	ps2pdf -dCompatibilityLevel=1.3 -dMaxSubsetPct=100 -dSubsetFonts=true \
# 	-dEmbedAllFonts=true -dAutoFilterColorImages=false \
# 	-dAutoFilterGrayImages=false -dColorImageFilter=/FlateEncode \
# 	-dGrayImageFilter=/FlateEncode -dMonoImageFilter=/FlateEncode \
# 	${TARGET}.ps ${TARGET}.pdf

${TARGET}.pdf :
	pdflatex ${TARGET}

bib:
	bibtex ${TARGET}

${TARGET}.bbl: ${TARGET}.bib
	latex ${TARGET}
# get the citations out of the bibliography
	bibtex ${TARGET}
# do it again in case there are out-of-order cross-references
	latex ${TARGET}

${TARGET}.dvi: ${TARGET}.bbl ${TARGET}.tex ${SOURCES}
	latex ${TARGET}

one:	*.tex
	pdflatex ${TARGET}

two:	*.tex
	pdflatex ${TARGET}
	pdflatex ${TARGET}

reb:	${DEPEND}
	pdflatex ${TARGET}
	bibtex ${TARGET}
	pdflatex ${TARGET}

# shortcut, so we can say "make ps"
ps: ${TARGET}.ps

${TARGET}.ps: ${TARGET}.dvi 
	@dvips -Ppdf -t letter -G0 -o ${TARGET}.ps ${TARGET}.dvi 

clean:
	rm -f ${TARGET}.{log,aux,ps,dvi,bbl,blg,log}

reallyclean: clean
	rm -f ${TARGET}.{ps,pdf}

view: ${TARGET}.pdf
	acroread ${TARGET}.pdf

zip:
	@if(test -e core) then rm core; fi
	@zip "$(BACKUPPATH)/$(PAPER)`date +%Y-%m-%d-%H.%M`.zip" eps/* *.tex Makef* *.bib *.sty *.cls *.fd bib/*.bib alg/*.tex img/*


zipall:
	@if(test -e core) then rm core; fi
	@zip "$(BACKUPPATH)/ALL$(PAPER)`date +%Y-%m-%d-%H.%M`.zip" eps/* *.tex Makef* *.bib *.sty *.cls *.fd bib/*.bib alg/*tex img/* figs/* figs/*/*


PHONY : ps all clean reallyclean

