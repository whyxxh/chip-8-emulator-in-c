#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <time.h>
#include "chip8.h"

#define TIMER_FREQUENCY 60 // Timer frequency in Hz
#define TIMER_INTERVAL (1000 / TIMER_FREQUENCY) // Interval in milliseconds
//
void decrementTimers(Chip8 *chip8) {
    if (chip8->dt > 0) {
        chip8->dt--;
    }
    if (chip8->st > 0) {
        chip8->st--;
    }
}

int checkKeyPress(Chip8 *chip8) {
    memset(chip8->kb, 0, sizeof(chip8->kb));
    if (IsKeyDown(KEY_ONE)) { chip8->kb[0x1] = 1; return 0x1;}
    if (IsKeyDown(KEY_TWO)) { chip8->kb[0x2] = 1; return 0x2;}
    if (IsKeyDown(KEY_THREE)) { chip8->kb[0x3] = 1; return  0x3;}
    if (IsKeyDown(KEY_FOUR)) { chip8->kb[0xC] = 1; return 0xC;}

    if (IsKeyDown(KEY_Q)) { chip8->kb[0x4] = 1; return 0x4; }
    if (IsKeyDown(KEY_W)) { chip8->kb[0x5] = 1; return 0x5; }
    if (IsKeyDown(KEY_E)) { chip8->kb[0x6] = 1; return 0x6; }
    if (IsKeyDown(KEY_R)) { chip8->kb[0xD] = 1; return 0xD; }

    if (IsKeyDown(KEY_A)) { chip8->kb[0x7] = 1; return 0x7; }
    if (IsKeyDown(KEY_S)) { chip8->kb[0x8] = 1; return 0x8; }
    if (IsKeyDown(KEY_D)) { chip8->kb[0x9] = 1; return 0x9; }
    if (IsKeyDown(KEY_F)) { chip8->kb[0xE] = 1; return 0xE; }

    if (IsKeyDown(KEY_Z)) { chip8->kb[0xA] = 1; return 0xA; }
    if (IsKeyDown(KEY_X)) { chip8->kb[0x0] = 1; return 0x0; }
    if (IsKeyDown(KEY_C)) { chip8->kb[0xB] = 1; return 0xB; }
    if (IsKeyDown(KEY_V)) { chip8->kb[0xF] = 1; return 0xF; }

    return 0;
}


void initChip8(Chip8 *chip8) {
    memset(chip8->ram, 0, sizeof(chip8->ram));
    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->screen, 0, sizeof(chip8->screen));
    memset(chip8->kb, 0, sizeof(chip8->kb));
    chip8->dt = 0;
    chip8->st = 0;
    chip8->op = 0;
    chip8->pc = 0x200;
    chip8->i = 0;
    chip8->sp = 0;
    for (int i = 0; i < 80; i++) {
        chip8->ram[0x50 + i] = FONTSET[i];
    }
}

int loadProgram(Chip8 *chip8, const char *file) {
    FILE *program = fopen(file, "rb");
    if (!program) {
        fprintf(stderr, "failed to load the program");
        return -1;
    }
    // reads the content of the binary file
    size_t bytesRead = fread(&chip8->ram[0x200], sizeof(u8), sizeof(chip8->ram) - 0x200, program);
    if (bytesRead == 0 && ferror(program)) {
        fprintf(stderr, "failed to read the program");
        return -1;
    }
    return 0;
}

void fetch(Chip8 *chip8) {
    u16 opcode;
    u8 firstByte = chip8->ram[chip8->pc];
    u8 secondByte = chip8->ram[chip8->pc + 1];
    opcode = secondByte + (firstByte << 8);
    chip8->pc += 2;
    chip8->op = opcode;
}

void execute(Chip8 *chip8) {
    u16 nnn = chip8->op & 0x0FFF;    // nnn value (last 12 bits)
    u8 x = (chip8->op & 0x0F00) >> 8; // x value (bits 8-11)
    u8 y = (chip8->op & 0x00F0) >> 4; // y value (bits 4-7)
    u8 nn = chip8->op & 0x00FF;       // nn value (last 8 bits)
    u8 n = chip8->op & 0x000F;        // n value (last 4 bits, used for sprite height)

    switch (chip8->op & 0xF000) {
        case 0x0000:
            switch (chip8->op & 0x00FF) {
                case CLS:
                    memset(&chip8->screen[0], 0, sizeof(chip8->screen));
                    break;
                case RET:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
            }
            break;

        case JMP:
            chip8->pc = nnn;
            break;

        case CAL:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = nnn;
            break;

        case LD:
            chip8->V[x] = nn;
            break;

        case LDI:
            chip8->i = nnn;
            break;

        case DRW: {
            u8 xCoord = chip8->V[x] % 64;
            u8 yCoord = chip8->V[y] % 32;
            chip8->V[0xF] = 0;
            for (int i = 0; i < n; i++) {
                u8 rowData = chip8->ram[chip8->i + i];
                for (int j = 0; j < 8; j++) {
                    if (xCoord + j < 64 && yCoord + i < 32) {
                        int idx = (yCoord + i) * 64 + (xCoord + j);
                        u8 spritePixel = (rowData >> (7 - j)) & 1;
                        if (spritePixel) {
                            if (chip8->screen[idx]) {
                                chip8->screen[idx] = 0;
                                chip8->V[0xF] = 1;
                            } else {
                                chip8->screen[idx] = 1;
                            }
                        }
                    }
                }
            }
            break;
        }

        case ADD:
            chip8->V[x] += nn;
            chip8->V[x] &= 0xFF;
            break;

        case SE:
            if (chip8->V[x] == nn) {
                chip8->pc += 2;
            }
            break;

        case SNE:
            if (chip8->V[x] != nn) {
                chip8->pc += 2;
            }
            break;

        case SEV:
            if (chip8->V[x] == chip8->V[y]) {
                chip8->pc += 2;
            }
            break;

        case SNEV:
            if (chip8->V[x] != chip8->V[y]) {
                chip8->pc += 2;
            }
            break;

        case 0x8000:
            switch (chip8->op & 0xF00F) {
                case MOV:
                    chip8->V[x] = chip8->V[y];
                    break;

                case OR:
                    chip8->V[x] |= chip8->V[y];
                    break;

                case AND:
                    chip8->V[x] &= chip8->V[y];
                    break;

                case XOR:
                    chip8->V[x] ^= chip8->V[y];
                    break;

                case ADDC: {
                    u16 sum  = (u16)chip8->V[x] + (u16)chip8->V[y];
                    chip8->V[x] = sum & 0xFF;
                    chip8->V[0xF] = (sum > 255) ? 1 : 0;
                    break;
                }

                case SUBY: {
                    u8 prevVx = chip8->V[x];
                    u8 prevVy = chip8->V[y]; 
                    chip8->V[x] = chip8->V[x] - chip8->V[y];
                    chip8->V[0xF] = (prevVx >= prevVy) ? 1 : 0;
                    break;
                }

                case SUBX: {
                    u8 prevVx = chip8->V[x];
                    u8 prevVy = chip8->V[y]; 
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0xF] = (prevVy >= prevVx) ? 1 : 0;
                    break;
                }

                case SHR: {
                    u8 lsb =  chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;     
                    chip8->V[0x0F] = lsb;
                    break;
                }

                case SHL: {
                    u8 msb = (chip8->V[x] >> 7) & 0x1;
                    chip8->V[x] <<= 1;
                    chip8->V[0xF] = msb;     
                    break;
                }
            }
            break;

        case JMPV:
            chip8->pc = nnn + chip8->V[0];
            break;

        case RND:
            chip8->V[x] = (rand() & 0xFF) & nn;
            break;

        case 0xE000:
            switch (chip8->op & 0xF0FF) {
                case SKP:
                    printf("checking for key %x,", chip8->V[x]);
                    printf("Key state for V[%d]: %d\n", x, chip8->kb[chip8->V[x]]);
                    if (chip8->kb[chip8->V[x]]) {
                        chip8->pc += 2;
                    }
                    break;

                case SKNP:
                    printf("checking for key %x,", chip8->V[x]);
                    printf("Key state for V[%d]: %d\n", x, chip8->kb[chip8->V[x]]);
                    if (!chip8->kb[chip8->V[x]]) {
                        chip8->pc += 2;
                    }
                    break;
            }
            break;

        case 0xF000:
            switch (chip8->op & 0xF0FF) {
                case GDT:
                    chip8->V[x] = chip8->dt;
                    break;

                case GKP:
                    chip8->key = 0;
                    while (1) {
                        decrementTimers(chip8);

                        chip8->key = checkKeyPress(chip8);

                        if (chip8->key > 0) {
                            chip8->V[x] = chip8->key;
                            break; 
                        }

                        if (WindowShouldClose()) {
                            break;
                        }
                    }
                    break;

                case SDT:
                    chip8->dt = chip8->V[x];
                    break;

                case SST:
                    chip8->st = chip8->V[x];
                    break;

                case ADI:
                    chip8->i += chip8->V[x];
                    break;

                case LDS:
                    chip8->i = 0x50 + (chip8->V[x] * 5);
                    break;

                case BCD:
                    chip8->ram[chip8->i] = chip8->V[x] / 100;
                    chip8->ram[chip8->i + 1] = (chip8->V[x] / 10) % 10;
                    chip8->ram[chip8->i + 2] = chip8->V[x] % 10;
                    break;

                case STM:
                    for (u16 i = 0; i <= x; i++) {
                        chip8->ram[chip8->i + i] = chip8->V[i];
                    }
                    break;

                case LDM:
                    for (int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->ram[chip8->i + i];
                    }
                    break;
            }
            break;
    }
}



void draw(Chip8 *chip8) {
    BeginDrawing();
    ClearBackground(BLACK);
    for (int i = 0; i < HEIGHT / SCALE; i++) {
        for (int j = 0; j < WIDTH / SCALE; j++) {
            int idx = j + (i * (WIDTH / SCALE));
            if (chip8->screen[idx]) {
                DrawRectangle(j * SCALE, i * SCALE, SCALE, SCALE, GREEN);
            }
        }
    }
    EndDrawing();
}



int main(int argc, char *argv[]) {
    srand(time(0));
    Chip8 chip8;
    initChip8(&chip8);

    if (argc < 2) {
        printf("Usage: %s <path_to_rom.ch8>\n", argv[0]);
        return -1;
    }

    if (loadProgram(&chip8, argv[1]) != 0) {
        printf("Failed to load ROM: %s\n", argv[1]);
        return -1;
    }

    // SetTargetFPS(120);
    clock_t lastTimerUpdate = clock();

    InitWindow(WIDTH, HEIGHT, "CHIP 8 EMU");

    while (!WindowShouldClose()) {
        chip8.key = checkKeyPress(&chip8);
        fetch(&chip8);
        execute(&chip8);
        draw(&chip8);

        clock_t currentTime = clock();
        if (((currentTime - lastTimerUpdate) * 1000 / CLOCKS_PER_SEC) >= TIMER_INTERVAL) {
            decrementTimers(&chip8);
            lastTimerUpdate = currentTime;
        }
    }

    CloseWindow(); // Close the window when done
    return 0;
}
