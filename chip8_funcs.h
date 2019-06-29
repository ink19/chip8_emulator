#ifndef CHIP8_FUNCS_H__
#define CHIP8_FUNCS_H__

#include "chip8_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int load_rom(chip8_emulator_t *emu, char *filename);
extern int main_loop(chip8_emulator_t *emu);
extern int emulator_destroy(chip8_emulator_t *emu);

#endif