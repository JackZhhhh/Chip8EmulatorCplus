#include "chip8.h"
#include <iostream>
#include <fstream>
#include <vector>

//3.1 Constructor and Initializer 
Chip8::Chip8() {
    initialize();
}

void Chip8::initialize() {
    pc = 0x200; // Programs start at address 0x200
    opcode = 0;
    I = 0;
    sp = 0;
    drawFlag = false;
    memory.fill(0);
    V.fill(0);
    stack.fill(0);
    gfx.fill(0);
    key.fill(0);

    delay_timer = 0;
    sound_timer = 0;

    //3.2 Fontset
    uint8_t fontset[80] = {
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

    for (int i = 0; i < 80; ++i)
        memory[i] = fontset[i];
}

//3.3 ROM Loader
void Chip8::loadROM(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open ROM\n";
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        for (size_t i = 0; i < buffer.size(); ++i)
            memory[0x200 + i] = buffer[i];
    }

    file.close();
}

void Chip8::emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: gfx.fill(0); drawFlag = true; pc += 2; break;
                case 0x00EE: --sp; pc = stack[sp]; pc += 2; break;
                default: std::cerr << "Unknown 0x0000 opcode: " << std::hex << opcode << "\n"; pc += 2; break;
            }
            break;

        case 0x1000: pc = opcode & 0x0FFF; break;
        case 0x2000: stack[sp] = pc; ++sp; pc = opcode & 0x0FFF; break;
        case 0x3000: if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) pc += 4; else pc += 2; break;
        case 0x4000: if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) pc += 4; else pc += 2; break;
        case 0x5000: if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) pc += 4; else pc += 2; break;
        case 0x6000: V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF; pc += 2; break;
        case 0x7000: V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF; pc += 2; break;

        case 0x8000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F) {
                case 0x0: V[x] = V[y]; break;
                case 0x1: V[x] |= V[y]; break;
                case 0x2: V[x] &= V[y]; break;
                case 0x3: V[x] ^= V[y]; break;
                case 0x4: {
                    uint16_t sum = V[x] + V[y];
                    V[0xF] = (sum > 255) ? 1 : 0;
                    V[x] = sum & 0xFF;
                    break;
                }
                case 0x5: {
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    break;
                }
                case 0x6: {
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                }
                case 0x7: {
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                }
                case 0xE: {
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    break;
                }
                default: std::cerr << "Unknown 0x8000 opcode: " << std::hex << opcode << "\n"; break;
            }
            pc += 2;
            break;
        }

        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) pc += 4;
            else pc += 2;
            break;

        case 0xA000: I = opcode & 0x0FFF; pc += 2; break;
        case 0xB000: pc = (opcode & 0x0FFF) + V[0]; break;
        case 0xC000: V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF); pc += 2; break;

        case 0xD000: {
            uint8_t x = V[(opcode & 0x0F00) >> 8];
            uint8_t y = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;
            V[0xF] = 0;

            for (int row = 0; row < height; ++row) {
                uint8_t sprite = memory[I + row];
                for (int col = 0; col < 8; ++col) {
                    if ((sprite & (0x80 >> col)) != 0) {
                        int index = (x + col + ((y + row) * 64)) % (64 * 32);
                        if (gfx[index] == 1) V[0xF] = 1;
                        gfx[index] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x9E: if (key[V[x]]) pc += 4; else pc += 2; break;
                case 0xA1: if (!key[V[x]]) pc += 4; else pc += 2; break;
                default: std::cerr << "Unknown 0xE000 opcode: " << std::hex << opcode << "\n"; pc += 2; break;
            }
            break;
        }

        case 0xF000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x07: V[x] = delay_timer; break;
                case 0x0A: {
                    bool key_pressed = false;
                    for (int i = 0; i < 16; ++i) {
                        if (key[i]) {
                            V[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) return; // Wait
                    break;
                }
                case 0x15: delay_timer = V[x]; break;
                case 0x18: sound_timer = V[x]; break;
                case 0x1E: I += V[x]; break;
                case 0x29: I = V[x] * 5; break;
                case 0x33:
                    memory[I] = V[x] / 100;
                    memory[I + 1] = (V[x] / 10) % 10;
                    memory[I + 2] = V[x] % 10;
                    break;
                case 0x55:
                    for (int i = 0; i <= x; ++i) memory[I + i] = V[i];
                    break;
                case 0x65:
                    for (int i = 0; i <= x; ++i) V[i] = memory[I + i];
                    break;
                default:
                    std::cerr << "Unknown 0xF000 opcode: " << std::hex << opcode << "\n";
                    break;
            }
            pc += 2;
            break;
        }

        default:
            std::cerr << "Unknown opcode: " << std::hex << opcode << "\n";
            pc += 2;
            break;
    }


    if (delay_timer > 0)
        --delay_timer;
    if (sound_timer > 0) {
        --sound_timer;
        if (sound_timer == 0)
            std::cout << "BEEP!\n";
    }
}
