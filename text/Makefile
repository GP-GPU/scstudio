MAIN=bc-en

DVI_PROGRAM=latex
DVI_PROGRAM_ARGUMENTY=
PDF_PROGRAM=pdflatex
PDF_PROGRAM_ARGUMENTY=
PDF_OPTIMALIZATOR=pdfopt
PDF_OPTIMALIZATOR_ARGUMENTY=
PS_PROGRAM=pdftops
PS_PROGRAM_ARGUMENTY=-paper match

#INDEXPROGRAM=csindex
INDEXPROGRAM=makeindex

all: pdf clean

pdf: ${MAIN}.pdf

.PHONY : all
	
${MAIN}.pdf : ${MAIN}.tex
	@echo -e "\n### Kompilujem PDF ###"
	${PDF_PROGRAM} ${PDF_PROGRAM_ARGUMENTY} ${MAIN}
	bibtex ${MAIN}
	${PDF_PROGRAM} ${PDF_PROGRAM_ARGUMENTY} ${MAIN}
	${PDF_PROGRAM} ${PDF_PROGRAM_ARGUMENTY} ${MAIN}
clean:
	@echo -e "\n### Cistim nepotrebne zvysky po jednotlivych dokumentoch... ###"
	-rm -f ${MAIN}.{aux,bbl,blg,log,out}
	@echo -e "### Cistenie hotovo. ###"

#all:
#	rm -f bc-template-en.{aux,log,toc}
#	pdflatex bc-template-en.tex
#	pdflatex bc-template-en.tex
#	mv bc-template-en.pdf bc.pdf
