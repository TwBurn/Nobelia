SHELL=/bin/bash

#Configuration, name, folders
NAME    = CDI_BOMB.APP
ELF     = cdi_bomb.elf
OUTPUT  = out
BUILD   = build
SRC     = src

#COMPILER CONFIGURATION
CC      = m68k-elfos9-gcc
CCFLAGS = -g -Wall -Wextra -march=68000 -mcpu=68000 -mpcrel -ma6rel -Os -fno-inline  -Wno-unused-parameter -Wno-implicit-function-declaration -Wno-unused-variable
AS      = m68k-elfos9-as
ASFLAGS = -g -march=68000 -mcpu=68000
LD      = m68k-elfos9-gcc
ELF2MOD = elf2mod
MASTER  = psxbuild
RM      = rm

#FILES TO COMPILE
FILES   = $(OUTPUT)/cstart.o $(OUTPUT)/sys.o $(OUTPUT)/intercept.o $(OUTPUT)/game.o $(OUTPUT)/weapons.o $(OUTPUT)/player.o $(OUTPUT)/main.o $(OUTPUT)/audio.o $(OUTPUT)/video.o $(OUTPUT)/graphics.o $(OUTPUT)/input.o $(OUTPUT)/object.o

#LINKER CONFIGURATION
LDSCRIPT = cdi.lds
LDPARAM = -Wl,-q -nostdlib -T $(LDSCRIPT)

cd: all
	$(MASTER) --cuefile build.cat NOBELIA.bin

all: force_build_date link $(BUILD)/MUSIC.RTF $(BUILD)/DATA.RTF

rebuild: clean cd

$(BUILD)/MUSIC.RTF: $(BUILD)/M00_GWLD.ACM $(BUILD)/M01_TRAV.ACM
	ruby acm2xa.rb $@ $+

$(BUILD)/DATA.RTF: $(BUILD)/TILES.BIN $(BUILD)/TITLE.BIN $(BUILD)/SFX.BIN
	dd if=$(BUILD)/TILES.BIN of=$@ bs=2048 conv=sync
	dd if=$(BUILD)/TITLE.BIN of=$@ bs=2048 conv=notrunc,sync oflag=append
	dd if=$(BUILD)/SFX.BIN of=$@ bs=2048 conv=notrunc,sync oflag=append

link: $(FILES) $(LDSCRIPT) | $(OUTPUT)
	$(LD) $(LDPARAM) -o $(OUTPUT)/$(ELF) $(FILES) -lc -lgcc
	$(ELF2MOD) $(OUTPUT)/$(ELF) $(BUILD)/$(NAME)

$(OUTPUT)/cstart.o : $(SRC)/cstart.s | $(OUTPUT)
	$(AS) $(ASFLAGS) -o $@ $<

$(OUTPUT)/intercept.o : $(SRC)/intercept.s | $(OUTPUT)
	$(AS) $(ASFLAGS) -o $@ $<

$(OUTPUT)/sys.o : $(SRC)/sys.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OUTPUT)/audio.o : $(SRC)/audio.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/audio.c

$(OUTPUT)/game.o : $(SRC)/game.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/game.c
	
$(OUTPUT)/graphics.o : $(SRC)/graphics.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/graphics.c

$(OUTPUT)/input.o : $(SRC)/input.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/input.c

$(OUTPUT)/main.o : $(SRC)/main.c | $(OUTPUT)
	echo "#define BUILD_VERSION \"BUILD $$(date "+%Y/%m/%d %H:%M:%S")\n\"" > src/build.h
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/main.c

$(OUTPUT)/object.o : $(SRC)/object.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/object.c

$(OUTPUT)/player.o : $(SRC)/player.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/player.c

$(OUTPUT)/video.o : $(SRC)/video.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/video.c

$(OUTPUT)/weapons.o : $(SRC)/weapons.c | $(OUTPUT)
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/weapons.c

$(OUTPUT):
	mkdir -p $(OUTPUT)

force_build_date:
	-@$(RM) $(OUTPUT)/main.o

clean:
	-@$(RM) $(OUTPUT)/*.o
	-@$(RM) $(OUTPUT)/$(ELF)
	-@$(RM) $(BUILD)/$(NAME)
	-@$(RM) $(BUILD)/DATA.RTF
	-@$(RM) $(BUILD)/MUSIC.RTF
	-@$(RM) NOBELIA.bin
	-@$(RM) NOBELIA.cue

