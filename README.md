# Nobelia

## CD-i Game by Jeffrey Janssen - nobelia@nmotion.nl

This is the Open Source version of Nobelia. 
- The source code is identical to the released version.
- Licensed music is not included in this version, all in-game music tracks are replaced by "The Traveller"
- The source code and assets in this repository are meant for personal and/or educational use only

### Controls

**Up/Down/Left/Right**
- Move character

**Button 1**
- Start game on titlescreen
- Place bomb (tile in front of you needs to be empty)
- Activate chests/switches
- Restart level after death

**Button 2**
- Detonate remote bombs (when you have this powerup)

**Button 3**
- Pause game

## Resources

All assets in the game were either used from Open resources or created/adapted by me.

- Titlescreen:
	- Image: http://clipart-library.com/clipart/348852.htm
	- Music: Grassy World by Matthew Pablo: https://opengameart.org/content/grassy-world-overture-8bitorchestral
- Level sprites:
	- Armando Montero (ArMM1998): https://opengameart.org/content/zelda-like-tilesets-and-sprites
- Level Music
	- The Traveller by Viktor Kraus: https://opengameart.org/content/the-traveller
- Character Sprites:
	- Lanea Zimmerman (Sharm), Evert, withthelove: https://opengameart.org/content/tiny-16-expanded-character-sprites

## Changelog
- Makefile has been updated to use the compiler/toolset available from https://github.com/TwBurn/cdi-sdk
- Use DOSBox to call `vcdmastr.exe` since this is a 16-bit application (see `master.bat`)