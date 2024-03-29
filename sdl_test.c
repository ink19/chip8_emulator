#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *win = NULL;
    win = SDL_CreateWindow("Hello", 0, 0, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *bmp;
    bmp = SDL_LoadBMP("./hello.bmp");
    SDL_Texture *tex;
    tex = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_FreeSurface(bmp);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex, NULL, NULL);
    //SDL_RenderDrawPoint(renderer, 20, 20);
    SDL_Rect test;
    test.h = 100;
    test.w = 100;
    test.x = 20;
    test.y = 20;
    // SDL_RenderDrawRect(renderer, &test);
    SDL_RenderFillRect(renderer, &test);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    printf("%d\n", keystates[SDL_SCANCODE_Z]);
    if(keystates[SDL_SCANCODE_Z]) {
        printf("Test\n");
    }
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    
    SDL_Quit();
    return 0;
}