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
            {
                for (int i = 0; i < 64 * 32; ++i)
                    gfx[i] = 0x0;
                programCounter += 2;
                break;
            }
            case 0x000E:
            {
                --stackPointer;
                programCounter = stack[stackPointer];
                break;
            }
            default: //UNKOWN WE'LL JUST IGNORE
                printf("Opcode not known or not implemented [0x0000]: 0x%X\n", opcode);
                break;
            }
            break;

        case 0x1000: //Jumps to address
        {
            programCounter = opcode & 0x0FFF;
            break;
        }
        case 0x2000: //Call subroutine
        {
            programCounter += 2;
            stack[stackPointer] = programCounter;
            ++stackPointer;
            programCounter = opcode & 0x0FFF;
            break;
        }
        case 0x3000: //If VX == to NN skip next instruction
        {
            if (cpuRegisters[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                programCounter += 4;
            else
                programCounter += 2;
            break;
        }
        case 0x4000: //If vx != to vy skip next line
        {
            if (cpuRegisters[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                programCounter += 4;
            else
                programCounter += 2;
            break;
        }
        case 0x5000: //If vx == to vy skip next line
        {
            if (cpuRegisters[(opcode & 0x0F00) >> 8] == cpuRegisters[(opcode & 0x00F0) >> 4])
                programCounter += 4;
            else
                programCounter += 2;
            break;
        }
        case 0x6000: //Set VX to NN
        {
            cpuRegisters[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            programCounter += 2;
            break;
        }
        case 0x7000: //Add NN to VX
        {
            cpuRegisters[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            programCounter += 2;
            break;
        }
        case 0x8000: //Whole lotta math with VX and VY
            switch (opcode & 0x000F) {
                case 0x0000: //Set VX to VY
                {
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0001:    //Set VX to the OR value of VX and VY
                {
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] | cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0002:    //Set VX to the AND value of VX and VY
                {
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] & cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0003:    //Set VX to the XOR value of VX and VY
                {
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] ^ cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0004:    //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
                {
                    if (cpuRegisters[(opcode & 0x00F0) >> 4] > (0xFF - cpuRegisters[(opcode & 0x0F00) >> 8]))
                        cpuRegisters[0xF] = 1; //Final register slot is used for carry
                    else
                        cpuRegisters[0xF] = 0;
                    cpuRegisters[(opcode & 0x0F00) >> 8] += cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0005:    //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                {
                    if (cpuRegisters[(opcode & 0x00F0) >> 4] > cpuRegisters[(opcode & 0x0F00) >> 8])
                        cpuRegisters[0xF] = 0;
                    else
                        cpuRegisters[0xF] = 1;
                    cpuRegisters[(opcode & 0x0F00) >> 8] -= cpuRegisters[(opcode & 0x00F0) >> 4];
                    programCounter += 2;
                    break;
                }

                case 0x0006:    //Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                {
                    cpuRegisters[0xF] = cpuRegisters[(opcode & 0x0F00) >> 8] & 0x1;
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] >> 1;
                    programCounter += 2;
                    break;
                }

                case 0x0007:
                {
                    if (cpuRegisters[(opcode & 0x00F0) >> 4] < cpuRegisters[(opcode & 0x0F00) >> 8])
                        cpuRegisters[0xF] = 0;
                    else
                        cpuRegisters[0xF] = 1;
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x00F0) >> 4] - cpuRegisters[(opcode & 0x0F00) >> 8];
                    programCounter += 2;
                    break;
                }

                case 0x000E:
                {
                    cpuRegisters[0xF] = cpuRegisters[(opcode & 0x0F00) >> 8] >> 7;
                    cpuRegisters[(opcode & 0x0F00) >> 8] = cpuRegisters[(opcode & 0x0F00) >> 8] << 1;
                    programCounter += 2;
                    break;
                }
            }
            break;
        case 0x9000:
        {
            if (cpuRegisters[(opcode & 0x0F00) >> 8] != cpuRegisters[(opcode & 0x00F0) >> 4])
                programCounter += 4;
            else
                programCounter += 2;
        }
            break;
        case 0xA000:
        {
            indexRegister = opcode & 0x0FFF;
            programCounter += 2;
            break;
        }
        case 0xB000:
        {
            programCounter = (opcode & 0x0FFF) + cpuRegisters[0];
            break;
        }
        case 0xC000:
        {
            cpuRegisters[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            programCounter += 2;
            break;
        }
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

        case 0xE000:
        {
            switch (opcode & 0x000F)
            {
                case 0x000E: //Skip next instruction if key stored in VX is pressed
                {
                    if (currentKey[cpuRegisters[(opcode & 0x0F00) >> 8]] != 0)
                        programCounter += 4;
                    else
                        programCounter += 2;
                    break;
                }
                   
                case 0x0001: //Skip next instruction if key stored in VX is not pressed
                {
                    if (currentKey[cpuRegisters[(opcode & 0x0F00) >> 8]] == 0)
                        programCounter += 4;
                    else
                        programCounter += 2;
                    break;
                }

                default:
                    break;
            }
        }

        case 0xF000:
        {
            switch (opcode & 0x00FF)
            {
                case 0x0007: //Sets VX to the value of the delay timer. 
                {
                    cpuRegisters[(opcode & 0x0F00) >> 8] = delayTimer;
                    programCounter += 2;
                    break;
                }
                case 0x000A: //A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event); 
                {
                    bool keyPress = false;

                    for (int i = 0; i < 16; ++i)
                    {
                        if (currentKey[i] != 0)
                        {
                            cpuRegisters[(opcode & 0x0F00) >> 8] = i;
                            keyPress = true;
                        }
                    }

                    // If we didn't received a keypress, skip this cycle and try again.
                    if (!keyPress)
                        return;

                    programCounter += 2;
                    break;
                }
                case 0x0015: // Sets the delay timer to VX.
                {
                    delayTimer = cpuRegisters[(opcode & 0x0F00) >> 8];
                    programCounter += 2;
                    break;
                }
                case 0x0018: // Sets the sound timer to VX. 
                {
                    soundTimer = cpuRegisters[(opcode & 0x0F00) >> 8];
                    programCounter += 2;
                    break;
                }
                case 0x001E: // Adds VX to I. VF is not affected.
                {
                    indexRegister += cpuRegisters[(opcode & 0x0F00) >> 8];
                    programCounter += 2;
                    break;
                }
                case 0x0029: // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. 
                {
                    indexRegister = cpuRegisters[(opcode & 0x0F00) >> 8] * 0x5;
                    programCounter += 2;
                    break;
                }
                case 0x0033: // Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
                {
                    memory[indexRegister] = cpuRegisters[(opcode & 0x0F00) >> 8] / 100;
                    memory[indexRegister + 1] = (cpuRegisters[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[indexRegister + 2] = (cpuRegisters[(opcode & 0x0F00) >> 8] % 100) % 10;
                    programCounter += 2;
                    break;
                }
                case 0x0055: // Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.					
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[indexRegister + i] = cpuRegisters[i];

                    // On the original interpreter, when the operation is done, I = I + X + 1.
                    indexRegister += ((opcode & 0x0F00) >> 8) + 1;
                    programCounter += 2;
                    break;

                case 0x0065: // Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.[					
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        cpuRegisters[i] = memory[indexRegister + i];

                    // On the original interpreter, when the operation is done, I = I + X + 1.
                    indexRegister += ((opcode & 0x0F00) >> 8) + 1;
                    programCounter += 2;
                    break;
                default:
                    break;
            }
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