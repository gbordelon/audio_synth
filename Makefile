TOPDIR	:= .
SRCDIR	:= $(TOPDIR)/src
OBJDIR	:= $(TOPDIR)/obj

CFILES	:= c
OFILES	:= o
CC		:= cc
CFLAGS	:= -std=c11 -pedantic -Wall -march=native -Ofast -g

SYNTH_C_NAMES	:= $(shell find . -name '*.c' ! -path './fx*')
FX_C_NAMES	:= $(shell find . -name '*.c' ! -path './synth*')
H_NAMES	:= $(shell find . -name '*.h')

SYNTH		:= $(TOPDIR)/synth
FX		:= $(TOPDIR)/fx_proc
DEPS	:= $(H_NAMES)
SYNTH_OBJECTS	:= $(shell sed -e 's/.\/src/obj/g' <<< " $(SYNTH_C_NAMES:.c=.o) " )
FX_OBJECTS	:= $(shell sed -e 's/.\/src/obj/g' <<< " $(FX_C_NAMES:.c=.o) " )

.PHONY: all alldefault clean synth fx

all: $(FX) $(SYNTH)

$(SYNTH): $(SYNTH_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lfftw3 -lportmidi -framework CoreAudio -framework CoreServices -framework AudioUnit -framework AudioToolBox -fno-pie

fx: $(FX)

$(FX): $(FX_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lfftw3 -lportmidi -framework CoreAudio -framework CoreServices -framework AudioUnit -framework AudioToolBox -fno-pie

$(OBJDIR)/%.$(OFILES): $(SRCDIR)/%.$(CFILES) $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SYNTH) $(FX) synth.o fx_proc.o
	rm -rf $(OBJDIR)
