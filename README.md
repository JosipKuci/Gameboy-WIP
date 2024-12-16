# Gameboy emulator(WIP)
As part of my Bachleor thesis, I am creating a Gameboy(DMG-01) emulator.
It is made using the C programming language and the SDL2 graphics library
It was compiled and tested using the 32-bit mingw32 compiler on a windows 10 machine 
### How to compile and run

To compile the code, go to the Gameboy directory and run the command:

`mingw32-make`

Then to start the emulator, change into the  `bin` directory and start the main.exe file by also specifying the ROM you want to play, for example:

`main.exe .\Tetris.gb`

### Note
This emulator is in its very early stage of development, currently I have only implemented
most of the instruction set and not much else, however you can see most of the logic in [gb_instruction_processing file](https://github.com/JosipKuci/Gameboy/blob/main/src/gb_instruction_processing.c)
It currently displays each instruction being executed, as well as all the registers and flags.
