INPUT = status.tex

all: status.pdf

status.pdf: $(INPUT)
	pdflatex $(INPUT)
	pdflatex $(INPUT)

clean:
	rm -f *.aux *.log *.bbl *.pdf
