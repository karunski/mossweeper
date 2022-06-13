# Minesweeper for NES and C64
## About

This is a clone of the game "minesweeper" for the NES and the C64. I created this game as a project learn about these
8-bit computers, to experiment with the capabilities of the LLVM-MOS (http://llvm-mos.org) toolchain.

I wrote this game to see if it was possible to create a real, playable game on an 8-bit computer using a modern language and tools.
This game is written entirely in C++, or more precisely, c++17.  Despite being written with modern tools, the game still fits in
the smallest-sized NES cartridge type, or < 20kB of space on C64.  And, in both cases, I'm not really close to using all the character
graphics (tiles) available.

## Features

1. Three Difficulty Levels (Beginner, Intermediate, Expert)
2. Colorful Graphics 
3. Sound Effects
4. Joystick / Controller controls
5. Keyboard controls where applicable.
6. The NES version is tested / working on actual NES hardware.
7. The C64 version has not been tested on an actual C64, since I do not own one.  I have tested it on VICE and on the MiSTer C64 core.

## Downloading and Running

Builds are provided under the "Releases" tab on this github project.  Just download the "rom" or 
program file and load into whatever hardware device or emulator you like.

## How to play

Move the tile selection cursor around the board using your computers directional controls, which could be a joystick or direction pad,
or cursor keys, where applicable. 

Select a tile to reveal from the board by using the fire button, or "button A" on the NES.  If the tile was over a mine, it's game over. 
If not, the number revealed will tell you how many mines are present in surrounding tiles. Use this information to deduce which 
tiles contain mines, or don't contain mines. You win the game by revealing all tiles that do not contain mines, without revealing a mine.

When you figure out that a tile has a mine under it, you can mark it with a flag. This is done by holding down the fire button, or using
'button B' on NES. 

You can reset the game board by moving the cursor up to the "smiley face" at the top of the game board, and pressing the fire button.

C64 Screenshot
![vice-screenshot-expert](https://user-images.githubusercontent.com/1659725/173267692-d6dd15c0-a485-4848-9367-b938b1e4fef5.png)

NES Screenshot
![minesweeper 6 2022-06-12 22 18 18](https://user-images.githubusercontent.com/1659725/173268126-85612b88-d84e-4266-89f2-9245b4d05341.png)

