#include "chip8.h"
#include<time.h>

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

chip8::chip8(std::FILE* ROM)
{
	rom = ROM;
}

void chip8::initialize()
{
	PC = 0x200; // it always starts at 0x200
	opcode = 0; // reset opcode
	IndexRegister = 0; // reset index register
	sp = 0; // reset stack pointer

	DelayTimer = 0;
	SoundTimer = 0;

	isDraw = true;

	// clear memory
	for (int i = 0; i < 4096; ++i)
		mem[i] = 0;

	// Load Fontset
	for (int i = 0; i < 80; ++i)
		mem[i] = chip8_fontset[i];

	// clear stack and registers
	for (int i = 0; i < 16; i++)
	{
		reg[i] = 0;
		stack[i] = 0;
	}

	// seed for opcode 0xCXNN
	srand(time(NULL));
}

void chip8::emulateCycle()
{
	unsigned char x, y;
	// the opcode is gotten by combining 2 chars
	opcode = mem[PC] << 8 | mem[PC + 1];

	// decode opcodes
	// some opcodes cant use this first statement
	// so we use other statements for those
	switch (opcode & 0xF000)
	{
		case 0x0000:
			switch (opcode & 0xF)
			{
				case 0x0000:
					for (int i = 0; i < 2048; i++)
						gfx[i] = 0;
					isDraw = true;
					PC += 2;
					//printf("Call\n");
					break;

				case 0x000E:
					sp--;
					PC = stack[sp];
					PC += 2;
					printf("Clear Screen\n");
					break;

				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			}
			break;

		case 0x1000:
			PC = opcode & 0x0FFF;
			printf("Jump to " + PC);
			printf("\n");
			break;

		case 0x2000:
			stack[sp] = PC;
			sp++;
			PC = opcode & 0xFFF;
			printf("Call to " + PC);
			printf("\n");
			break;

		case 0x3000:
			if (reg[(opcode & 0xF00) >> 8] == opcode & 0xFF)
				PC += 4; // skip next instruction
			else
				PC += 2; // goto next instruction

			printf("Skip if VX == NN\n");
			break;

		case 0x4000:
			if (reg[(opcode & 0xF00) >> 8] != opcode & 0xFF)
				PC += 4; // skip next instruction
			else
				PC += 2; // goto next instruction

			printf("Skip if VX != NN\n");
			break;

		case 0x5000:
			x = reg[(opcode & 0xF00 >> 8)];
			y = reg[(opcode & 0xF0 >> 4)];
			if (x == y)
				PC += 4;
			else
				PC += 2;

			printf("Skip is VX == VY\n");
			break;

		case 0x6000:
			reg[(opcode & 0xF00 >> 8)] = opcode & 0x00FF;
			PC += 2;

			printf("VX == NN\n");
			break;

		case 0x7000:
			reg[(opcode & 0xF00 >> 8)] += opcode & 0x00FF;
			PC += 2;

			printf("VX += NN\n");
			break;

		case 0x8000:
			switch (opcode & 0xF)
			{
				case 0x0000:
					reg[(opcode & 0xF00 >> 8)] = reg[(opcode & 0xF0) >> 4];
					PC += 2;

					printf("VX == VY\n");
					break;

				case 0x0001:
					reg[(opcode & 0xF00 >> 8)] |= reg[(opcode & 0xF0) >> 4];
					PC += 2;

					printf("VX = VX | VY\n");
					break;

				case 0x0002:
					reg[(opcode & 0xF00 >> 8)] &= reg[(opcode & 0xF0) >> 4];
					PC += 2;

					printf("VX = VX & VY\n");
					break;

				case 0x0003:
					reg[(opcode & 0xF00 >> 8)] ^= reg[(opcode & 0xF0) >> 4];
					PC += 2;
					printf("VX = VX ^ VY\n");
					break;

				// these next cases change the carry value (register 16)
				case 0x0004:
					if (reg[(opcode & 0xF0) >> 4] > (0xFF - reg[(opcode & 0xF00) >> 8]))
						reg[0xF] = 1;
					else
						reg[0xF] = 0;

					reg[(opcode & 0xF00) >> 8] += reg[(opcode & 0xF0) >> 4];
					PC += 2;
					break;

				case 0x0005:
					if (reg[(opcode & 0xF0) >> 4] > reg[(opcode & 0xF00) >> 8])
						reg[0xF] = 0;
					else
						reg[0xF] = 1;
					reg[(opcode & 0xF00) >> 8] -= reg[(opcode & 0xF0) >> 4];
					PC += 2;
					break;

				case 0x0006:
					reg[0xF] = reg[(opcode & 0xF00) >> 8] & 0x01;
					y = reg[(opcode & 0xF0) >> 4];
					reg[(opcode & 0xF00) >> 8] = y >> 1;
					PC += 2;
					break;

				case 0x0007:
					if (reg[(opcode & 0xF00) >> 8] > reg[(opcode & 0xF0) >> 4])
						reg[0xF] = 0;
					else
						reg[0xF] = 1;
					reg[(opcode & 0xF00) >> 8] = reg[(opcode & 0xF0) >> 4] - reg[(opcode & 0xF00) >> 8];
					PC += 2;
					break;

				case 0x000E:
					reg[0xF] = reg[(opcode & 0xF0) >> 4] >> 7;
					y = reg[(opcode & 0xF0 >> 4)] << 1;
					reg[(opcode & 0xF00) >> 8] = y;
					PC += 2;
					break;
			}
			break;

		case 0x9000:
			x = reg[(opcode & 0xF00) >> 8];
			y = reg[(opcode & 0xF0) >> 4];

			if (x != y)
				PC += 4;
			else
				PC += 2;
			break;

		case 0xA000:
			IndexRegister = opcode & 0x0FFF;
			PC += 2;
			break;

		case 0xB000:
			PC = (opcode & 0x0FFF) + reg[0];
			break;

		case 0xC000:
			reg[(opcode & 0xF00) >> 8] = rand() & (opcode & 0xFF);
			PC += 2;
			break;

		case 0xD000:
		{
			x = reg[(opcode & 0xF00) >> 8];
			y = reg[(opcode & 0xF0) >> 4];
			unsigned short height = opcode & 0xF;
			unsigned short pixel;

			reg[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = mem[IndexRegister + yline];

				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
							reg[0xF] = 1;

						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			isDraw = true;
			PC += 2;
		}
		break;

		case 0xE000:
			switch (opcode & 0xFF)
			{
				case 0x009E:
					if (key[reg[(opcode & 0xF00) >> 8]] != 0)
						PC += 4;
					else
						PC += 2;
					break;

				case 0xA1:
					if (key[reg[(opcode & 0xF00) >> 8]] == 0)
						PC += 4;
					else
						PC += 2;
					break;
			}
			break;

		case 0x000F:
			switch (opcode & 0xFF)
			{
				case 0x0007:
					reg[(opcode & 0xF00) >> 8] = DelayTimer;
					PC += 2;
					break;

				case 0x000A:
				{
					printf("waiting for button press...");
					// so in this case a key press is awaited.
					// loop until one of the key entries is pressed
					bool pressed = false;

					for (int i = 0; i < 16; i++)
					{
						if (key[i] != 0)
						{
							// get x
							reg[(opcode & 0xF00) >> 8] = i;
							pressed = true;
						}
					}
					// keep going if there was no press
					if (!pressed)
						return;

					PC += 2;
				}
			    break;

				case 0x0015:
					DelayTimer = reg[(opcode & 0xF00) >> 8];
					PC += 2;
					break;

				case 0x0018:
					SoundTimer = reg[(opcode & 0xF00) >> 8];
					PC += 2;
					break;

				case 0x001E:
					IndexRegister += reg[(opcode & 0xF00) >> 8];
					PC += 2;
					break;

				case 0x0029:
					IndexRegister = reg[(opcode & 0xF00) >> 8] * 5;
					PC += 2;
					break;

				case 0x0033:
					mem[IndexRegister] = reg[(opcode & 0xF00) >> 8] / 100;
					mem[IndexRegister + 1] = (reg[(opcode & 0x0F00) >> 8] / 10) % 10;
					mem[IndexRegister + 2] = (reg[(opcode & 0xF00) >> 8] % 100) % 10;
					PC += 2;
					break;

				case 0x0055:				
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
						mem[IndexRegister + i] = reg[i];

					IndexRegister += ((opcode & 0xF00) >> 8) + 1;
					PC += 2;
					break;

				case 0x0065:				
					for (int i = 0; i <= ((opcode & 0xF00) >> 8); i++)
						reg[i] = mem[IndexRegister + i];

					IndexRegister += ((opcode & 0xF00) >> 8) + 1;
					PC += 2;
					break;
			}
			break;

		default:
			printf("Unknown opcode 0x%X\n", opcode);
	}

	if (DelayTimer > 0)
		--DelayTimer;

	if (SoundTimer > 0)
	{
		if (SoundTimer == 1)
			printf("BEEP!\n");
		--SoundTimer;
	}
}