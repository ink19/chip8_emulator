#ifndef CHIP8_TYPES_H__
#define CHIP8_TYPES_H__

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//操作码类型
typedef unsigned short opencode_t;

//内存类型
typedef unsigned char memory_t;

//寄存器类型
typedef unsigned char chip8_register_t;

//计时器类型
typedef unsigned char chip8_timer_t;

//模拟器
typedef struct {
    struct {
        SDL_Window *window;
        SDL_Renderer *renderer;
    } window;

    //内存
    memory_t memory[4096];
    
    //寄存器
    struct {
        chip8_register_t V[16];
        unsigned short I;
        unsigned short PC;
    } register_data;

    //图像显示
    unsigned char graph_view[64 * 32];
    
    //计时器
    struct {
        chip8_timer_t delay_timer;
        chip8_timer_t sound_timer;
    } timer;

    //栈
    struct {
        unsigned short data[16];
        int SP;
    } stack;

    unsigned char keys[16];
} chip8_emulator_t;

/**
 * key map
 * 1 -> 1
 * 2 -> 2
 * 3 -> 3
 * 4 -> 4
 * 5 -> 5
 * 6 -> 6
 * 7 -> 7
 * 8 -> 8
 * 9 -> 9
 * 0 -> 0
 * q -> a
 * w -> b
 * e -> c
 * a -> d
 * s -> e
 * d -> f
*/

#endif