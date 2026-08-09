MAIN := main
BUILD := build

.PHONY: all clean pendientes

all:
	mkdir -p $(BUILD)
	latexmk -pdf -bibtex -interaction=nonstopmode -halt-on-error \
		-outdir=$(BUILD) $(MAIN).tex
	cp $(BUILD)/$(MAIN).pdf informe.pdf

pendientes:
	@grep -RIn --exclude-dir=build --include='*.tex' '\\pendiente{' secciones || true

clean:
	latexmk -C -outdir=$(BUILD) $(MAIN).tex 2>/dev/null || true
	rm -f informe.pdf
