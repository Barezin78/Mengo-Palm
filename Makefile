# Mengão para Palm OS 5 — build com prc-tools-remix + pilrc
SDK      ?= sdk/sdk-5r3/include
INCS     := $(shell find $(SDK) -type d -printf "-I%p ")
CC       := m68k-palmos-gcc
CFLAGS   := -O2 -Wall -Wno-multichar -Ibuild $(INCS)
CREATOR  := MNGO
# Um lugar só: a linha VERSION em src/mengao.rcp (ex.: VERSION 1 "1.1.0")
VERSION  := $(shell sed -n 's/^VERSION 1 "\([^"]*\)".*/\1/p' src/mengao.rcp)
ifeq ($(strip $(VERSION)),)
$(error Não achei a linha VERSION em src/mengao.rcp)
endif

all: dist/Mengao.prc dist/Mengao-v$(VERSION).prc

build/%.c: src/%.c | build
	iconv -f UTF-8 -t CP1252 $< > $@
build/mengao.rcp: src/mengao.rcp | build
	iconv -f UTF-8 -t CP1252 $< > $@
build/resource.h: src/resource.h | build
	cp $< $@

build/main.o: build/main.c build/resource.h
	$(CC) $(CFLAGS) -c $< -o $@
build/mengao: build/main.o
	$(CC) $(CFLAGS) $^ -o $@

build/icons.stamp: tools/make_icons.py | build
	python3 tools/make_icons.py build && touch $@
build/res.stamp: build/mengao.rcp build/resource.h build/icons.stamp
	cd build && rm -f *.bin && pilrc -q -I . mengao.rcp > /dev/null && touch res.stamp

dist/Mengao.prc: build/mengao build/res.stamp | dist
	build-prc -o $@ -n Mengao -c $(CREATOR) build/mengao build/*.bin

# Mesmo PRC, com a versão no nome do arquivo. O banco interno continua "Mengao".
dist/Mengao-v$(VERSION).prc: dist/Mengao.prc
	cp -f $< $@

data: | dist
	python3 tools/gerar_dados.py dist/MengaoDados.pdb
	cp -f dist/MengaoDados.pdb "dist/MengaoDados-$$(date +%Y-%m-%d).pdb"

build dist:
	mkdir -p $@
clean:
	rm -rf build dist/Mengao.prc dist/Mengao-v*.prc
.PHONY: all data clean
