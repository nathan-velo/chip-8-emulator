#pragma once

class chip8
{
    private:
        unsigned short opcode;
        unsigned char memory[4096];
        unsigned char cpuRegisters[16];
        unsigned short indexRegister;
        unsigned short programCounter;
        unsigned char delayTimer;
        unsigned char soundTimer;
        unsigned short stack[16];
        unsigned short stackPointer;
    
    public:
        chip8(/* args */);

        void emulateCycle();
        void initialize();

        bool drawFlag;

        unsigned char currentKey[16];
        unsigned char gfx[64 * 32];

        bool loadFile(const char * filename);
};
