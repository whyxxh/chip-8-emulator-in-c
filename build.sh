#!/usr/bin/env bash

gcc src/chip8.c -o chip -Wall -Wextra -I$(brew --prefix raylib)/include -L$(brew --prefix raylib)/lib -lraylib -lm -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
