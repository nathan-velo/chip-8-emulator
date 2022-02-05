#include "chip8.h"


chip8 programChip;

int main(int argc, char **argv){
    programChip.loadFile(argv[1]);
}