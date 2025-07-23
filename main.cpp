// main.cpp
#include "chip8.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <thread>

const int VIDEO_SCALE = 10;
const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: chip8 <ROM file>\n";
        return 1;
    }

    Chip8 chip8;
    chip8.loadROM(argv[1]);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        VIDEO_WIDTH * VIDEO_SCALE, VIDEO_HEIGHT * VIDEO_SCALE,
        SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        VIDEO_WIDTH, VIDEO_HEIGHT);

    bool quit = false;
    SDL_Event event;

    const uint8_t keymap[16] = {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3,
        SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c,
        SDLK_4, SDLK_r, SDLK_f, SDLK_v
    };

    while (!quit) {
        // Handle input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                bool pressed = (event.type == SDL_KEYDOWN);
                for (int i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym == keymap[i])
                        chip8.key[i] = pressed;
                }
            }
        }

        chip8.emulateCycle();

        // Draw
        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            uint32_t pixels[VIDEO_WIDTH * VIDEO_HEIGHT];
            for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; ++i) {
                uint8_t pixel = chip8.gfx[i];
                pixels[i] = pixel ? 0xFFFFFFFF : 0xFF000000;
            }

            SDL_UpdateTexture(texture, nullptr, pixels, VIDEO_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(16667)); // ~60Hz
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
