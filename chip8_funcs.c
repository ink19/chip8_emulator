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
    memset(emu->keys, 0, sizeof(unsigned char) * 16);
    emu->stack.SP = 0;
    emu->register_data.I = 0;
    emu->register_data.PC = 0x200;
    memset(emu->register_data.V, 0, sizeof(chip8_register_t) * 16);
    emu->timer.delay_timer = 0;
    emu->timer.sound_timer = 0;

    memcpy(emu->memory, chip8_fontset, sizeof(memory_t) * 80);
    FILE *filep = fopen(filename, "rb");
    fread(emu->memory + 0x200, sizeof(memory_t), 4096 - 0x200, filep);
    fclose(filep);
    //IO交互
    SDL_Init(SDL_INIT_EVERYTHING);
    emu->window.window = SDL_CreateWindow(filename, 100, 100, 640, 320, SDL_WINDOW_SHOWN);
    emu->window.renderer = SDL_CreateRenderer(emu->window.window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    return 0;
}

static int load_opcode(chip8_emulator_t *emu, unsigned short *opcode) {
    memory_t temp[2];
    memcpy(temp, emu->memory + emu->register_data.PC, sizeof(unsigned char) * 2);
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
    --(emu->stack.SP);
    //printf("OUT b %d\n", emu->register_data.PC);
    emu->register_data.PC = emu->stack.data[(emu->stack.SP)];
    //printf("OUT a %d\n", emu->register_data.PC);
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
    emu->stack.data[emu->stack.SP] = emu->register_data.PC;
    (emu->stack.SP)++;
    //printf("IN b %d\n", emu->register_data.PC);
    emu->register_data.PC = opcode;
    //printf("IN a %d\n", emu->register_data.PC);
    return 0;
}

static int opcode_3(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff)) {
        emu->register_data.PC += 2;
        //printf("E %d %d %d\n", (opcode & 0x0f00) >> 8, (opcode & 0x00ff), emu->register_data.PC - 4);
    }
    //printf("%d %d\n", emu->register_data.V[(opcode & 0x0f00) >> 8], emu->register_data.PC - 4);
    return 0;
}

static int opcode_4(chip8_emulator_t *emu, unsigned short opcode) {
    if(emu->register_data.V[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff)) {
        emu->register_data.PC += 2;
        //printf("NE %d %d %d\n", (opcode & 0x0f00) >> 8, (opcode & 0x00ff), emu->register_data.PC - 4);
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
            if((unsigned int)(emu->register_data.V[(opcode & 0x0f00) >> 8]) + (unsigned int)(emu->register_data.V[(opcode & 0x00f0) >> 4]) > 0xff) {
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
    emu->register_data.V[0xf] = 0;
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
    //const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    
    if(emu->keys[emu->register_data.V[(opcode & 0x0f00) >> 8]] == 1) {
        state = 1;
        //printf("Test\n");
    }
    if((opcode & 0x00ff) == 0x009e) {
        if(state == 1) {
            emu->register_data.PC += 2;
            //printf("E %d \n", emu->register_data.V[(opcode & 0x0f00) >> 8]);
        }
    } else if((opcode & 0x00ff) == 0x00a1) {
        if(state == 0) {
            emu->register_data.PC += 2;
            //printf("NE %d \n", emu->register_data.V[(opcode & 0x0f00) >> 8]);
        }
    }
    return 0;
}

static int opcode_F(chip8_emulator_t *emu, unsigned short opcode) {
    unsigned short registerV = (opcode & 0x0f00) >> 8;
    int key = 0, getKey = 0;
    opcode &= 0x00ff;
    switch (opcode)
    {
    case 0x07:
        emu->register_data.V[registerV] = emu->timer.delay_timer;
        break;
    case 0x0a:

        for(key = 0; key < 16; ++key) {
            if(emu->keys[key] == 1) {
                emu->register_data.V[registerV] = key;
                getKey = 1;
                break;
            }
        }
        if(getKey == 0) emu->register_data.PC -= 2;
        //printf("%d\n", emu->register_data.PC);
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
    SDL_Rect points[64 * 32];
    int point_number = 0;
    for(int loop_i = 0; loop_i < 64 * 32; loop_i++) {
        if(emu->graph_view[loop_i] == 1) {
            points[point_number].x = loop_i % 64 * 10;
            points[point_number].y = loop_i / 64 * 10;
            points[point_number].h = 10;
            points[point_number].w = 10;
            ++point_number;
        } 
    }
    SDL_Rect RectAll;
    RectAll.x = 0;
    RectAll.y = 0;
    RectAll.h = 320;
    RectAll.w = 640;
    SDL_SetRenderDrawColor(emu->window.renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(emu->window.renderer, &RectAll);
    SDL_SetRenderDrawColor(emu->window.renderer, 0x0, 0x0, 0x0, 0xff);
    SDL_RenderFillRects(emu->window.renderer, points, point_number);
    SDL_RenderPresent(emu->window.renderer);
    return 0;
}

extern int main_loop(chip8_emulator_t *emu) {
    unsigned short opcode;
    SDL_Event event;
    int gameover = 0;
    clock_t start, final;
    final = clock();
    while(!gameover) {
        start = final;
        const Uint8 *keystates = SDL_GetKeyboardState(NULL);
        if(keystates[SDL_SCANCODE_ESCAPE]) {
            break;
        }
        memset(emu->keys, 0, sizeof(unsigned char) * 16);
        if(keystates[SDL_SCANCODE_0]) {
            emu->keys[0] = 1;
        }
        if(keystates[SDL_SCANCODE_1]) {
            emu->keys[1] = 1;
        }
        if(keystates[SDL_SCANCODE_2]) {
            emu->keys[2] = 1;
        }
        if(keystates[SDL_SCANCODE_3]) {
            emu->keys[3] = 1;
        }
        if(keystates[SDL_SCANCODE_4]) {
            emu->keys[4] = 1;
        }
        if(keystates[SDL_SCANCODE_5]) {
            emu->keys[5] = 1;
        }
        if(keystates[SDL_SCANCODE_6]) {
            emu->keys[6] = 1;
        }
        if(keystates[SDL_SCANCODE_7]) {
            emu->keys[7] = 1;
        }
        if(keystates[SDL_SCANCODE_8]) {
            emu->keys[8] = 1;
        }
        if(keystates[SDL_SCANCODE_9]) {
            emu->keys[9] = 1;
        }
        if(keystates[SDL_SCANCODE_Q]) {
            emu->keys[10] = 1;
        }
        if(keystates[SDL_SCANCODE_W]) {
            emu->keys[11] = 1;
        }
        if(keystates[SDL_SCANCODE_E]) {
            emu->keys[12] = 1;
        }
        if(keystates[SDL_SCANCODE_A]) {
            emu->keys[13] = 1;
        }
        if(keystates[SDL_SCANCODE_S]) {
            emu->keys[14] = 1;
        }
        if(keystates[SDL_SCANCODE_D]) {
            emu->keys[15] = 1;
        }
        load_opcode(emu, &opcode);
        exec_opcode(emu, opcode);
        refresh_timer(emu);
        draw_view(emu);
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                gameover = 1;
            } 
        }
        //printf("Over\n");
        
        // for(int loop_i = 0; loop_i < 16; ++loop_i) {
        //     printf("%d", emu->keys[loop_i]);
        // }
        // printf("\n");
        //printf("%d\n", emu->register_data.PC - 0x200);
        final = clock();
        if((start - final) < CLOCKS_PER_SEC/60) {
            //printf("%d \n", 1000/60 - (start - final) * 1000 / CLOCKS_PER_SEC);
            SDL_Delay(1000/60 - (start - final) * 1000 / CLOCKS_PER_SEC);
        }
    }
    return 0;
}

extern int emulator_destroy(chip8_emulator_t *emu) {
    SDL_DestroyRenderer(emu->window.renderer);
    SDL_DestroyWindow(emu->window.window);
    SDL_Quit();
    return 0;
}