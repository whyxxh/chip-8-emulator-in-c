#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
  Chip8 *chip8;
  int x = 0;
  chip8->V[x] = 123;
  chip8->ram[chip8->i] = chip8->V[x] / 100;
  chip8->ram[chip8->i + 1] = (chip8->V[x] / 10) % 10;
  chip8->ram[chip8->i + 2] = chip8->V[x] % 10;
  printf("%d %d %d", chip8->ram[chip8->i], chip8->ram[chip8->i + 1], chip8->ram[chip8->i + 2]);
  return EXIT_SUCCESS;
}
