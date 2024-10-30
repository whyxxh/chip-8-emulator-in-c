build:
	gcc src/chip8.c -o chip -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
