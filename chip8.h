// main cpu
#include<iostream>

class chip8
{
public:
	chip8(std::FILE*);
	void initialize();
	void emulateCycle();

	unsigned short opcode; // current opcode
	unsigned char mem[4096]; // 4K memory storage
	unsigned char reg[16]; // 16 registers
	unsigned short stack[16]; // stack size
	unsigned short sp; // stack pointer

	unsigned short IndexRegister; // index register
	unsigned short PC; // program counter

	unsigned char gfx[2048]; // 64 x 32 pixels
	unsigned char DelayTimer, SoundTimer; // timers
	unsigned char key[16]; // key state

	bool isDraw;

	std::FILE* rom;

};