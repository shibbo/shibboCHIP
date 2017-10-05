#include<iostream>
#include <stdio.h>  
#include "chip8.h"

int main()
{
	FILE* rom;
	void RunROM(FILE*);

	int err = fopen_s(&rom, "pong.rom", "rb");

	if (err == 0)
	{
		printf("File was opened.");
		RunROM(rom);
	}
	else
		printf("File not found. Aborting...");

	return 0;
}

void RunROM(FILE* rom)
{
	// make a new instance
	chip8 chip = chip8(rom);
	// set up registers, stack pointers, etc
	chip.initialize();

	for (;;)
	{
		chip.emulateCycle();
	}

	fclose(rom);
}