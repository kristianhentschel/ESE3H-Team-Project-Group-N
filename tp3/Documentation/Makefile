

TeamN.pdf: TeamN.tex citations.bib
	pdflatex TeamN
	bibtex TeamN
	pdflatex TeamN
	pdflatex TeamN

clean:
	rm -f *.log *.aux *.toc *.out
	rm -f *.bbl *.blg
	rm -f TeamN.pdf
	
