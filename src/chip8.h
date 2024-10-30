#include <stdint.h>
#include <raylib.h>

//screen
#define WIDTH 640
#define HEIGHT 320
#define SCALE 10

// timer
#define TIMER_FREQUENCY 60                // Timer frequency in Hz
#define TIMER_INTERVAL (1000 / TIMER_FREQUENCY) // Interval in milliseconds

// instructions
#define CLS 0x00E0  // Clear the display.
#define RET 0x00EE  // Return from subroutine.
#define JMP 0x1000  // Jump to address nnn.
#define CAL 0x2000  // Call subroutine at nnn.
#define SE 0x3000   // Skip next instruction if Vx == kk.
#define SNE 0x4000  // Skip next instruction if Vx != kk.
#define SEV 0x5000  // Skip next instruction if Vx == Vy.
#define LD 0x6000   // Set Vx = kk.
#define ADD 0x7000  // Set Vx = Vx + kk.
#define MOV 0x8000  // Set Vx = Vy.
#define OR 0x8001   // Set Vx = Vx OR Vy.
#define AND 0x8002  // Set Vx = Vx AND Vy.
#define XOR 0x8003  // Set Vx = Vx XOR Vy.
#define ADDC 0x8004 // Set Vx = Vx + Vy, set VF = carry.
#define SUBY 0x8005  // Set Vx = Vx - Vy, set VF = NOT borrow.
#define SHR 0x8006  // Set Vx = Vx SHR 1.
#define SUBX 0x8007 // Set Vx = Vy - Vx, set VF = NOT borrow.
#define SHL 0x800E  // Set Vx = Vx SHL 1.
#define SNEV 0x9000 // Skip next instruction if Vx != Vy.
#define LDI 0xA000  // Set I = nnn.
#define JMPV 0xB000 // Jump to nnn + V0.
#define RND 0xC000  // Set Vx = random byte AND kk.
#define DRW 0xD000  // Draw sprite at (Vx, Vy), set VF = collision.
#define SKP 0xE09E  // Skip next instruction if key Vx is pressed.
#define SKNP 0xE0A1 // Skip next instruction if key Vx is not pressed.
#define GDT 0xF007  // Set Vx = delay timer.
#define GKP 0xF00A  // Wait for key press, store in Vx.
#define SDT 0xF015  // Set delay timer = Vx.
#define SST 0xF018  // Set sound timer = Vx.
#define ADI 0xF01E  // Set I = I + Vx.
#define LDS 0xF029  // Set I = location of sprite for digit Vx.
#define BCD 0xF033  // Store BCD representation of Vx in memory at I.
#define STM 0xF055  // Store V0 through Vx in memory starting at I.
#define LDM 0xF065  // Load V0 through Vx from memory starting at I.

typedef uint8_t u8;
typedef uint16_t u16;

typedef struct {
    u16 opcode;
    const char* mnemonic;
} Instruction;

const u8 FONTSET[] = {
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

typedef struct {
  u8 ram[4096];
  u16 stack[16];
  u8 V[16];
  u16 pc;
  u8 sp;
  u16 i;
  u8 dt;
  u8 st;
  u16 op;
  int screen[64 * 32];
  u8 kb[0xF];
  u8 key;
} Chip8;


