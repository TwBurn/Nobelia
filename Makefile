#Configuration, name, folders
NAME    = nobelia
OUTPUT  = out
BUILD   = build
SRC     = src

#COMPILER AND LIBRARY LOCATIONS
CDISDK = D:\Development\Git\cdi-sdk
OS9C   = $(CDISDK)\dos\bin
OS68K  = $(CDISDK)\OS9\68000
OS9CDI = $(CDISDK)\OS9\CDI
XCLIB  = $(OS68K)\lib
XCDEF  = $(OS68K)\defs
CLIB   = $(OS9CDI)\lib
CDEF   = $(OS9CDI)\defs
PATH   = $(OS9C);%PATH%
DOSBOX = C:\Software\DOSBox-0.74-3\DOSBox.exe
#COMPILER CONFIGURATION
CC      = xcc
CCFLAGS = -eas=$(OUTPUT) -tp=68K,sc -v=$(CDEF) -bc -r
ASFLAGS = -O=0 -S -R=$(OUTPUT) -T=$(OUTPUT) -TO=osk -TP=68kI
LD      = l68

#FILES TO COMPILE
FILES   = $(OUTPUT)\main.r $(OUTPUT)\intro.r $(OUTPUT)\cdio.r $(OUTPUT)\audio.r $(OUTPUT)\video.r $(OUTPUT)\graphics.r $(OUTPUT)\input.r $(OUTPUT)\game.r $(OUTPUT)\title.r $(OUTPUT)\object.r $(OUTPUT)\player.r $(OUTPUT)\enemy.r $(OUTPUT)\skelet.r $(OUTPUT)\scripts.r $(OUTPUT)\weapons.r

#LINKER CONFIGURATION
LDPARAM = -a -n=cdi_$(NAME) -o=$(BUILD)\$(NAME) $(CLIB)\cstart.r $(FILES) -l=$(CLIB)\cdi.l -l=$(CLIB)\cdisys.l -l=$(CLIB)\clib.l -l=$(CLIB)\cio.l -l=$(CLIB)\math.l -l=$(CLIB)\sys.l -l=$(CLIB)\usr.l -l=$(XCLIB)\os_csl.l

cd: link
	$(DOSBOX) master.bat -exit

all: link

rebuild: clean cd

link: $(FILES)
	crt_link $(LDPARAM)
	$(LD) -z=link.txt
	fixmod -uo=0.0 $(BUILD)\$(NAME)

$(OUTPUT)\audio.r : $(SRC)\audio.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\audio.c

$(OUTPUT)\cdio.r : $(SRC)\cdio.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\cdio.c

$(OUTPUT)\enemy.r : $(SRC)\enemy.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\enemy.c

$(OUTPUT)\game.r : $(SRC)\game.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\game.c
	
$(OUTPUT)\graphics.r : $(SRC)\graphics.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\graphics.c

$(OUTPUT)\input.r : $(SRC)\input.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\input.c

$(OUTPUT)\intro.r : $(SRC)\intro.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\intro.c

$(OUTPUT)\main.r : $(SRC)\main.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\main.c

$(OUTPUT)\object.r : $(SRC)\object.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\object.c

$(OUTPUT)\player.r : $(SRC)\player.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\player.c

$(OUTPUT)\scripts.r : $(SRC)\scripts.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\scripts.c

$(OUTPUT)\skelet.r : $(SRC)\skelet.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\skelet.c

$(OUTPUT)\title.r : $(SRC)\title.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\title.c

$(OUTPUT)\video.r : $(SRC)\video.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\video.c

$(OUTPUT)\weapons.r : $(SRC)\weapons.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\weapons.c

clean:
	-@erase $(OUTPUT)\cm*
	-@erase $(OUTPUT)\*.tmp
	-@erase $(OUTPUT)\*.r
	-@erase $(BUILD)\$(NAME)

purge:
	-@erase $(OUTPUT)\cm*
	-@erase $(OUTPUT)\*.tmp
