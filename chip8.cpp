#include "chip8.h"
#include <stdio.h>
#include <stdlib.h> 

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

chip8::chip8() {
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char cpuRegisters[16];
    unsigned short indexRegister;
    unsigned short programCounter;
    unsigned char gfx[64 * 32];
    unsigned char delayTimer;
    unsigned char soundTimer;
    unsigned short stack[16];
    unsigned short stackPointer;
    unsigned char currentKey[16];
}

void chip8::initialize() {
    programCounter = 0x200;
    opcode = 0;
    indexRegister = 0;
    stackPointer = 0;

    for(int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];	
}

void chip8::emulateCycle() {
    //opcode is 2 bytes long so get 2 bytes from memory at current PC location
    opcode = memory[programCounter] << 8 | memory[programCounter + 1];


    //Decode opcode using https://johnearnest.github.io/Octo/docs/chip8ref.pdf as a reference
    //At first we only care about the first 4 bits and can add further statements for additional opcodes if necessary
    switch(opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F)
            {
            case 0x0000: //0x00E0 CLEAR SCREEN
                for(int i =0; i< 64 * 32; ++i)
                    gfx[i] = 0x0;
                break;
            case 0x000E:
                --stackPointer;
                programCounter = stack[stackPointer];
                programCounter += 2;
                break;
            default: //UNKOWN WE'LL JUST IGNORE
                printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
                break;
            }
            break;
        case 0x1000: //Jumps to address
            programCounter = opcode & 0x0FFF;
            break;
        case 0x2000: //Call subroutine
            programCounter += 2;
            stack[stackPointer] = programCounter;
            ++stackPointer;
            programCounter = opcode & 0x0FFF;
            break;
        case 0x3000: //If VX == to NN skip next instruction
            if(cpuRegisters[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                programCounter += 4;
            else
                programCounter += 2;
            break;
        case 0x4000: //If vx != to vy skip next line
            if(cpuRegisters[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                programCounter += 4;
            else
                programCounter += 2;
            break;
        case 0x5000: //If vx == to vy skip next line
            if(cpuRegisters[(opcode & 0x0F00) >> 8] == cpuRegisters[(opcode & 0x00F0) >> 4])
                programCounter += 4;
            else
                programCounter += 2;
            break;
        case 0x6000: //Set VX to NN
            cpuRegisters[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            programCounter += 2;
            break;
        case 0x7000: //Add NN to VX
            cpuRegisters[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            programCounter += 2;
            break;
        case 0x8000: //Whole lotta math with VX and VY
            switch (opcode & 0x000F) {
                case 0x0000: //Set VX to VY
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                case 0x0001: //Set VS to the OR value of VX and VY
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] | cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                case 0x0002:
                    break;
                case 0x0003:
                    break;
                case 0x0004:
                    break;
                case 0x0005:
                    break;
                case 0x0006:
                    break;
                case 0x0007:
                    break;
                case 0x000E:
                    break;
            }
    }
}



bool chip8::loadFile(const char * filename){
    initialize();
	printf("Loading: %s\n", filename);
		
	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr); 
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) 
	{
		fputs ("Memory error", stderr); 
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) 
	{
		fputs("Reading error",stderr); 
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");
	
	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;

}