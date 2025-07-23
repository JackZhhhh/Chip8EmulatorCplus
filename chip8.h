//2.2 Libraries, Class, & Public Access
#include <array>
#include <cstdint>

class Chip8 {
public:
    //2.5 Functions
    Chip8();               // Constructor
    void loadROM(const char* filename);  // Load a game
    void emulateCycle();   // Do one instruction cycle

private:
    //2.3 Array for Features
    std::array<uint8_t, 4096> memory; // 4K memory
    std::array<uint8_t, 2048> gfx; // 64x32 monochrome display
    std::array<uint8_t, 16> V;        // 16 general-purpose registers
    std::array<uint8_t, 16> key;      // HEX keypad (0x0 to 0xF)
    std::array<uint16_t, 16> stack;   // Call stack

    //2.4 Other Features
    uint16_t opcode;                  // Current instruction
    uint16_t sp;                      // Stack pointer  
    uint8_t delay_timer;             // Delay timer
    uint8_t sound_timer;             // Sound timer
    uint16_t I;                       // Index register
    uint16_t pc;                      // Program counter

    //2.5 Functions
    void initialize(); // Reset emulator state
};