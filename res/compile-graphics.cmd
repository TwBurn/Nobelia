@SET I2C=ImageToClut.exe
@SET OUTPUT_FOLDER=Nobelia\build
%I2C% %~dp0titlescreen.png %OUTPUT_FOLDER%\title.bin -p palette.json -a
%I2C% %~dp0titlepaper.png %OUTPUT_FOLDER%\paper.bin -p palette.json -a
%I2C% %~dp0finalscreen.png %OUTPUT_FOLDER%\score.bin -p palette.json -a
%I2C% %~dp0instructions.png %OUTPUT_FOLDER%\instruct.bin -p palette.json -a
%I2C% %~dp0spritesheet.png %OUTPUT_FOLDER%\sprites.bin -p palette.json -a
%I2C% %~dp0tilesheet.png %OUTPUT_FOLDER%\tiles_s.bin -p palette.json -a
%I2C% %~dp0discdirty.png %OUTPUT_FOLDER%\discerr.bin -p palette.json -a -f Rle7
%I2C% %~dp0twburn.png %OUTPUT_FOLDER%\twburn.bin -p palette.json -a -f Rle7