TOPDIR	:= .
SRCDIR	:= $(TOPDIR)/src
OBJDIR	:= $(TOPDIR)/obj

CFILES	:= c
OFILES	:= o
CC		:= cc
CFLAGS	:= -std=c11 -pedantic -Wall -g -march=native -O2

C_NAMES	:= $(shell find . -name '*.c')
H_NAMES	:= $(shell find . -name '*.h')

EXE		:= $(TOPDIR)/synth
DEPS	:= $(H_NAMES)
OBJECTS	:= $(shell sed -e 's/.\/src/obj/g' <<< " $(C_NAMES:%.c=%.o) " )

.PHONY: all alldefault clean exe

exe: $(EXE)


$(EXE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lportmidi -framework CoreAudio -framework CoreServices -framework AudioUnit -framework AudioToolBox

$(OBJDIR)/%.$(OFILES): $(SRCDIR)/%.$(CFILES) $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXE) main.o
	rm -rf $(OBJDIR)
