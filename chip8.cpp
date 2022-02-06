#include "chip8.h"
#include <stdio.h>
#include <stdlib.h> 
#include <chrono>
#include <thread>


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
    std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
}


void chip8::initialize() {
    programCounter = 0x200;
    opcode = 0;
    indexRegister = 0;
    stackPointer = 0;

    for(int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];	

    // Clear display
    for (int i = 0; i < 2048; ++i)
        gfx[i] = 0;

    // Clear stack
    for (int i = 0; i < 16; ++i)
        stack[i] = 0;

    for (int i = 0; i < 16; ++i)
        currentKey[i] = cpuRegisters[i] = 0;

    // Clear memory
    for (int i = 0; i < 4096; ++i)
        memory[i] = 0;

    // Load fontset
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delayTimer = 0;
    soundTimer = 0;

    // Clear screen once
    drawFlag = true;
}

void chip8::emulateCycle() {
    //opcode is 2 bytes long so get 2 bytes from memory at current PC location
    opcode = memory[programCounter] << 8 | memory[programCounter + 1];
    printf("%02x\n", (unsigned int)opcode);

    //Decode opcode using https://johnearnest.github.io/Octo/docs/chip8ref.pdf as a reference
    //Also using https://en.wikipedia.org/wiki/CHIP-8#Opcode_table as it has more detailed information
    //At first we only care about the first 4 bits and can add further statements for additional opcodes if necessary
    switch(opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F)
            {
            case 0x0000: //0x00E0 CLEAR SCREEN
                for(int i =0; i< 64 * 32; ++i)
                    gfx[i] = 0x0;
                programCounter += 2;
                break;

            case 0x000E:
                --stackPointer;
                programCounter = stack[stackPointer];
                break;

            default: //UNKOWN WE'LL JUST IGNORE
                printf("Opcode not known or not implemented [0x0000]: 0x%X\n", opcode);
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

                case 0x0001:    //Set VX to the OR value of VX and VY
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] | cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;

                case 0x0002:    //Set VX to the AND value of VX and VY
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] & cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;

                case 0x0003:    //Set VX to the XOR value of VX and VY
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] ^ cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;

                case 0x0004:    //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
                    if(cpuRegisters[(opcode & 0x00F0) >> 4] > (0xFF - cpuRegisters[(opcode & 0x0F00) >> 8])) 
						cpuRegisters[0xF] = 1; //Final register slot is used for carry
					else 
						cpuRegisters[0xF] = 0;	
                    cpuRegisters[(opcode & 0x0F00) >> 8] += cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;

                case 0x0005:    //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                    if(cpuRegisters[(opcode & 0x00F0) >> 4] > cpuRegisters[(opcode & 0x0F00) >> 8])
						cpuRegisters[0xF] = 0;
					else 
						cpuRegisters[0xF] = 1;	
                    cpuRegisters[(opcode & 0x0F00) >> 8] -= cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;

                case 0x0006:    //Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    cpuRegisters[0xF] = cpuRegisters[(opcode & 0x0F00) >> 8] & 0x1;
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] >> 1;
                    programCounter += 2;
                    break;

                case 0x0007:
                    if(cpuRegisters[(opcode & 0x00F0) >> 4] < cpuRegisters[(opcode & 0x0F00) >> 8])
						cpuRegisters[0xF] = 0;
					else 
						cpuRegisters[0xF] = 1;	
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x00F0) >> 4] - cpuRegisters[(opcode & 0x0F00) >> 8];
                    programCounter += 2;
                    break;

                case 0x000E:
                    cpuRegisters[0xF] = cpuRegisters[(opcode & 0x0F00) >> 8] >> 7;
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] << 1;
                    programCounter += 2;
                    break;
            }
            break;
        case 0x9000:
            if(cpuRegisters[(opcode & 0x0F00) >> 8] != cpuRegisters[(opcode & 0x00F0) >> 4])
                programCounter += 4;
            else
                programCounter += 2;
            break;
        case 0xA000:
            indexRegister = opcode & 0x0FFF;
            programCounter += 2;
            break;
        case 0xB000:
            programCounter = (opcode & 0x0FFF) + cpuRegisters[0];
            break;
        case 0xC000:
            cpuRegisters[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            programCounter += 2;
            break;
        case 0xD000:
        {
            unsigned short x = cpuRegisters[(opcode & 0x0F00) >> 8];
            unsigned short y = cpuRegisters[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            cpuRegisters[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[indexRegister + yline];
                for (int xline = 0; xline < 8; xline++)
                {
                    if ((pixel & (0x80 >> xline)) != 0)
                    {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            cpuRegisters[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            programCounter += 2;
            break;
        }
        default:
        {
            printf("Opcode not known or not implemented [0x0000]: 0x%X\n", opcode);
            break;
        }
        break;
    }

    if (delayTimer > 0)
        --delayTimer;

    if (soundTimer > 0)
    {
        if (soundTimer == 1)
        printf("BEEP!\n");
        --soundTimer;
    }

    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_time = a - b;

    const int instructionsPerSecond = 700;

    if (work_time.count() < 1000.0 / instructionsPerSecond)
    {
        std::chrono::duration<double, std::milli> delta_ms(1000.0 / instructionsPerSecond - work_time.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }

    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;

    // Your code here

    printf("Time: %f \n", (work_time + sleep_time).count());
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