#include "chip8_funcs.h"

unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

extern int load_rom(chip8_emulator_t *emu, char *filename) {
    //复位
    srand((unsigned)time(NULL));
    memset(emu->memory, 0, 4096 * sizeof(memory_t));
    memset(emu->graph_view, 0, sizeof(unsigned char) * 64 * 32);
    memset(emu->stack.data, 0, sizeof(memory_t) * 16);
    emu->stack.SP = 0;
    emu->register_data.I = 0;
    emu->register_data.PC = 0x200;
    memset(emu->register_data.V, 0, sizeof(chip8_register_t));
    emu->timer.delay_timer = 0;
    emu->timer.sound_timer = 0;

    memcpy(emu->memory, chip8_fontset, sizeof(memory_t) * 80);
    FILE *filep = fopen(filename, "rb");
    fread(emu->memory + 0x200, sizeof(memory_t), 4096, filep);
    fclose(filep);
    //IO交互
    SDL_Init(SDL_INIT_EVERYTHING);
    emu->window.window = SDL_CreateWindow(filename, 100, 100, 64, 32, SDL_WINDOW_SHOWN);
    emu->window.renderer = SDL_CreateRenderer(emu->window.window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    return 0;
}

static int load_opcode(chip8_emulator_t *emu, unsigned short *opcode) {
    memory_t temp[2];
    memcpy(emu->memory + emu->register_data.PC, temp, sizeof(unsigned char) * 2);
    emu->register_data.PC += 2;
    *opcode = ((unsigned short) temp[0]) << 8 | temp[1];
    return 0;
}

//清屏
static int disp_clear(chip8_emulator_t *emu) {
    memset(emu->graph_view, 0, sizeof(unsigned char) * 64 * 32);
    return 0;
}

static int return_op(chip8_emulator_t *emu) {
    emu->register_data.PC = emu->stack.data[--(emu->stack.SP)];
    return 0;
}

static int opcode_0(chip8_emulator_t *emu, unsigned short opcode) {
    switch (opcode)
    {
    case 0x00e0:
        disp_clear(emu);
        break;
    case 0x00ee:
        return_op(emu);
        break;
    default:
        break;
    }
}

static int opcode_1(chip8_emulator_t *emu, unsigned short opcode) {
    opcode &= 0x0fff;
    emu->register_data.PC = opcode;
}

static int opcode_2(chip8_emulator_t *emu, unsigned short opcode) {
    opcode &= 0x0fff;
    emu->stack.data[emu->stack.SP++] = emu->register_data.PC;
    emu->register_data.PC = opcode;
    return 0;
}

static int opcode_3(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff)) {
        emu->register_data.PC += 2;
    }
    return 0;
}

static int opcode_4(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff)) {
        emu->register_data.PC += 2;
    }
    return 0;
}

static int opcode_5(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] == emu->register_data.V[(opcode & 0x00f0) >> 4]) {
        emu->register_data.PC += 2;
    }
    return 0;
}

static int opcode_6(chip8_emulator_t *emu, unsigned short opcode) {
    emu->register_data.V[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
    return 0;
}

static int opcode_7(chip8_emulator_t *emu, unsigned short opcode) {
    emu->register_data.V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
    return 0;
}

static int opcode_8(chip8_emulator_t *emu, unsigned short opcode) {
    switch(opcode & 0x000f) {
        case 0x0:
            emu->register_data.V[(opcode & 0x0f00) >> 8] = emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x1:
            emu->register_data.V[(opcode & 0x0f00) >> 8] |= emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x2:
            emu->register_data.V[(opcode & 0x0f00) >> 8] &= emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x3:
            emu->register_data.V[(opcode & 0x0f00) >> 8] ^= emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x4:
            if((int)(emu->register_data.V[(opcode & 0x0f00) >> 8]) + (int)(emu->register_data.V[(opcode & 0x00f0) >> 4]) > 0xff) {
                emu->register_data.V[0xf] = 1;
            } else {
                emu->register_data.V[0xf] = 0;
            }
            emu->register_data.V[(opcode & 0x0f00) >> 8] += emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x5:
            if((int)(emu->register_data.V[(opcode & 0x0f00) >> 8]) > (int)(emu->register_data.V[(opcode & 0x00f0) >> 4])) {
                emu->register_data.V[0xf] = 1;
            } else {
                emu->register_data.V[0xf] = 0;
            }
            emu->register_data.V[(opcode & 0x0f00) >> 8] -= emu->register_data.V[(opcode & 0x00f0) >> 4];
        break;
        case 0x6:
            emu->register_data.V[0xf] = emu->register_data.V[(opcode & 0x0f00) >> 8] & 0x1;
            emu->register_data.V[(opcode & 0x0f00) >> 8] >>= 1;
        break;
        case 0x7:
            if((int)(emu->register_data.V[(opcode & 0x0f00) >> 8]) <= (int)(emu->register_data.V[(opcode & 0x00f0) >> 4])) {
                emu->register_data.V[0xf] = 1;
            } else {
                emu->register_data.V[0xf] = 0;
            }
            emu->register_data.V[(opcode & 0x0f00) >> 8] = emu->register_data.V[(opcode & 0x00f0) >> 4] - emu->register_data.V[(opcode & 0x0f00) >> 8];
        break;
        case 0xe:
            emu->register_data.V[0xf] = (emu->register_data.V[(opcode & 0x0f00) >> 8] & 0x80) >> 7;
            emu->register_data.V[(opcode & 0x0f00) >> 8] <<= 1;
        break;
    }
    return 0;
}

static int opcode_9(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] != emu->register_data.V[(opcode & 0x00f0) >> 4]) {
        emu->register_data.PC += 2;
    }
    return 0;
}

static int opcode_A(chip8_emulator_t *emu, unsigned short opcode) {
    opcode &= 0x0fff;
    emu->register_data.I = opcode;
    return 0;
}

static int opcode_B(chip8_emulator_t *emu, unsigned short opcode) {
    opcode &= 0x0fff;
    emu->register_data.PC = emu->register_data.V[0] + opcode;
    return 0;
}

static int opcode_C(chip8_emulator_t *emu, unsigned short opcode) {
    emu->register_data.V[(opcode & 0x0f00) >> 8] = (rand() % 256) & (opcode & 0x00ff);
    return 0;
}

static int opcode_D(chip8_emulator_t *emu, unsigned short opcode) {
    unsigned short x = emu->register_data.V[(opcode & 0x0f00) >> 8];
    unsigned short y = emu->register_data.V[(opcode & 0x00f0) >> 4];
    unsigned short I = emu->register_data.I;
    unsigned short N = opcode & 0x000f;
    unsigned char x_line = 0;
    for (int loop_y = 0; loop_y < N; ++loop_y) {
        x_line = emu->memory[I];
        for (int loop_x = 0; loop_x < 8; ++loop_x) {
            if(x_line& (0x80 >> loop_x)) {
                if(emu->graph_view[x + loop_x + (y + loop_y) * 64]) {
                    emu->register_data.V[0xf] = 1;
                }
                emu->graph_view[x + loop_x + (y + loop_y) * 64] ^= 1;
            }
        }
        I++;
    }
    return 0;
}

static int opcode_E(chip8_emulator_t *emu, unsigned short opcode) {
    int state = 0;
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    switch (emu->register_data.V[(opcode & 0x0f00) >> 8])
    {
    case 0x000a:
        if(keystates[SDLK_q]) {
            state = 1;
        }
        break;
    case 0x000b:
        if(keystates[SDLK_w]) {
            state = 1;
        }
        break;
    case 0x000c:
        if(keystates[SDLK_e]) {
            state = 1;
        }
        break;
    case 0x000d:
        if(keystates[SDLK_a]) {
            state = 1;
        }
        break;
    case 0x000e:
        if(keystates[SDLK_s]) {
            state = 1;
        }
        break;
    case 0x000f:
        if(keystates[SDLK_d]) {
            state = 1;
        }
        break;
    default:
        if(keystates[SDLK_0 + emu->register_data.V[(opcode & 0x0f00) >> 8]]) {
            state = 1;
        }
        break;
    }
    if(opcode & 0x00ff == 0x009e) {
        if(state == 1) {
            emu->register_data.PC += 2;
        }
    } else if(opcode & 0x00ff == 0x00a1) {
        if(state == 0) {
            emu->register_data.PC += 2;
        }
    }
    return 0;
}

static int opcode_F(chip8_emulator_t *emu, unsigned short opcode) {
    unsigned short registerV = (opcode & 0x0f00) >> 8;
    int key = 0;
    opcode &= 0x00ff;
    switch (opcode)
    {
    case 0x07:
        emu->register_data.V[registerV] = emu->timer.delay_timer;
        break;
    case 0x0a:
        
        while(1) {
            const Uint8 *keystates = SDL_GetKeyboardState(NULL);
            if(keystates[SDLK_0]) {
                key = 0;
                break;
            }
            if(keystates[SDLK_1]) {
                key = 1;
                break;
            }
            if(keystates[SDLK_2]) {
                key = 2;
                break;
            }
            if(keystates[SDLK_3]) {
                key = 3;
                break;
            }
            if(keystates[SDLK_3]) {
                key = 3;
                break;
            }
            if(keystates[SDLK_4]) {
                key = 4;
                break;
            }
            if(keystates[SDLK_5]) {
                key = 5;
                break;
            }
            if(keystates[SDLK_6]) {
                key = 6;
                break;
            }
            if(keystates[SDLK_7]) {
                key = 7;
                break;
            }
            if(keystates[SDLK_8]) {
                key = 8;
                break;
            }
            if(keystates[SDLK_9]) {
                key = 9;
                break;
            }
            if(keystates[SDLK_q]) {
                key = 0xa;
                break;
            }
            if(keystates[SDLK_w]) {
                key = 0xb;
                break;
            }
            if(keystates[SDLK_e]) {
                key = 0xc;
                break;
            }
            if(keystates[SDLK_a]) {
                key = 0xd;
                break;
            }
            if(keystates[SDLK_s]) {
                key = 0xe;
                break;
            }
            if(keystates[SDLK_d]) {
                key = 0xf;
                break;
            }
            SDL_Delay(20);
        }
        emu->register_data.V[registerV] = key;
        break;
    case 0x15:
        emu->timer.delay_timer = emu->register_data.V[registerV];
        break;
    case 0x18:
        emu->timer.sound_timer = emu->register_data.V[registerV];
        break;
    case 0x1e:
        emu->register_data.I += emu->register_data.V[registerV];
        break;
    case 0x29:
        emu->register_data.I = emu->register_data.V[registerV] * 5;
    break;
    case 0x33:
        emu->memory[emu->register_data.I] = emu->register_data.V[registerV] / 100;
        emu->memory[emu->register_data.I + 1] = (emu->register_data.V[registerV] / 10) % 10;
        emu->memory[emu->register_data.I + 2] = emu->register_data.V[registerV] % 10;
    break;
    case 0x55:
        for(int loop_r = 0; loop_r <= registerV; ++loop_r) {
            emu->memory[emu->register_data.I + loop_r] = emu->register_data.V[loop_r];
        }
    break;
    case 0x65:
        for(int loop_r = 0; loop_r <= registerV; ++loop_r) {
            emu->register_data.V[loop_r] = emu->memory[emu->register_data.I + loop_r];
        }
    break;
    
    default:
        break;
    }
}

static int exec_opcode(chip8_emulator_t *emu, unsigned short opcode) {
    int (*opcode_funs[])(chip8_emulator_t *emu, unsigned short opcode) = {
        opcode_0, opcode_1, opcode_2, opcode_3, opcode_4, opcode_5, opcode_6, opcode_7, opcode_8, opcode_9, opcode_A, opcode_B, opcode_C, opcode_D, opcode_E, opcode_F
    };

    (*(opcode_funs[(opcode & 0xf000) >> 12]))(emu, opcode);
    return 0;
}

static int refresh_timer(chip8_emulator_t *emu) {
    if(emu->timer.delay_timer > 0) emu->timer.delay_timer--;
    if(emu->timer.sound_timer > 0) emu->timer.sound_timer--;
    return 0;
}

static int draw_view(chip8_emulator_t *emu) {
    SDL_Point points[64 * 32];
    int point_number = 0;
    for(int loop_i = 0; loop_i < 64 * 32; loop_i++) {
        if(emu->graph_view[loop_i] == 1) {
            points[point_number].x = loop_i % 64;
            points[point_number].y = loop_i / 64;
            ++point_number;
        } 
    }
    SDL_RenderClear(emu->window.renderer);
    SDL_RenderDrawPoints(emu->window.renderer, points, point_number);
    SDL_RenderPresent(emu->window.renderer);
    return 0;
}

extern int main_loop(chip8_emulator_t *emu) {
    unsigned short opcode;
    while(1) {
        load_opcode(emu, &opcode);
        exec_opcode(emu, opcode);
        refresh_timer(emu);
        draw_view(emu);
        const Uint8 *keystates = SDL_GetKeyboardState(NULL);
        if(keystates[SDLK_ESCAPE]) {
            break;
        }
        SDL_Delay(15);
    }
}

extern int emulator_destroy(chip8_emulator_t *emu) {
    SDL_DestroyRenderer(emu->window.renderer);
    SDL_DestroyWindow(emu->window.window);
    SDL_Quit();
    return 0;
}