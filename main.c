#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8_funcs.h"
chip8_emulator_t emu;
int main() {
    
    load_rom(&emu, "./game.ch8");
    main_loop(&emu);
    emulator_destroy(&emu);
    return 0;
}