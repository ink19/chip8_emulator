#ifndef CHIP8_TYPES_H__
#define CHIP8_TYPES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//操作码类型
typedef unsigned short opencode_t;

//内存类型
typedef unsigned char memory_t;

//寄存器类型
typedef unsigned char register_t;

//计时器类型
typedef unsigned char timer_t;

//模拟器
typedef struct {
    //内存
    memory_t memory[4096];
    
    //寄存器
    struct {
        register_t V[16];
        register_t I;
        register_t PC;
    } register_data;

    //图像显示
    unsigned char graph_view[64 * 32];
    
    //计时器
    struct {
        timer_t delay_timer;
        timer_t sound_timer;
    } timer;

    //栈
    struct {
        memory_t data[16];
        int SP;
    } stack;
} chip8_emulator_t;

#endif