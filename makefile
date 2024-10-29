build:
	gcc src/chip8.c -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
