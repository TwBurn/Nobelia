#Configuration, name, folders
NAME    = cdi_bomb
OUTPUT  = out
BUILD   = build
SRC     = src

#COMPILER AND LIBRARY LOCATIONS
OS9C    = C:\CDI\OS9C
CLIB    = $(OS9C)\lib
MTOOLS  = C:\CDI\MASTER
PATH    = $(OS9C)\bin;$(MTOOLS);%PATH%

#COMPILER CONFIGURATION
CC      = xcc
CCFLAGS = -S -R=$(OUTPUT) -T=$(OUTPUT) -TO=osk -TP=68kI
ASFLAGS = -O=0 -S -R=$(OUTPUT) -T=$(OUTPUT) -TO=osk -TP=68kI
LD      = l68
MASTER  = master.exe

#FILES TO COMPILE
FILES   = $(OUTPUT)\game.r $(OUTPUT)\weapons.r $(OUTPUT)\player.r $(OUTPUT)\main.r $(OUTPUT)\audio.r $(OUTPUT)\video.r $(OUTPUT)\graphics.r $(OUTPUT)\input.r $(OUTPUT)\object.r

#LINKER CONFIGURATION
LDPARAM = -n=$(NAME) -o=$(BUILD)\$(NAME) $(CLIB)\cstart.r $(FILES) -l=$(CLIB)\cdi.l -l=$(CLIB)\cdisys.l -l=$(CLIB)\clib.l -l=$(CLIB)\cio.l -l=$(CLIB)\math.l -l=$(CLIB)\sys.l -l=$(CLIB)\usr.l

cd: all
	$(MASTER) build.cd

all: force_build_date link

rebuild: clean cd

link: $(FILES)
	crt_link $(LDPARAM)
	$(LD) -z=link.txt
	fixmod -uo=0.0 $(BUILD)\$(NAME)

$(OUTPUT)\audio.r : $(SRC)\audio.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\audio.c

$(OUTPUT)\game.r : $(SRC)\game.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\game.c
	
$(OUTPUT)\graphics.r : $(SRC)\graphics.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\graphics.c

$(OUTPUT)\input.r : $(SRC)\input.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\input.c

$(OUTPUT)\main.r : $(SRC)\main.c
	set_build
	$(CC) $(CCFLAGS) -O=2 $(SRC)\main.c

$(OUTPUT)\object.r : $(SRC)\object.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\object.c

$(OUTPUT)\player.r : $(SRC)\player.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\player.c

$(OUTPUT)\video.r : $(SRC)\video.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\video.c

$(OUTPUT)\weapons.r : $(SRC)\weapons.c
	$(CC) $(CCFLAGS) -O=2 $(SRC)\weapons.c

force_build_date:
	-@erase $(OUTPUT)\main.r

clean:
	-@erase $(OUTPUT)\*.r
	-@erase $(BUILD)\$(NAME)

