BIN = snake

# compiler
CC = clang

# includes and libs
INCS = $(shell pkg-config --cflags sdl3)
LIBS = $(shell pkg-config --libs sdl3)

# flags
CFLAGS = $(INCS) -O2 -std=c18 -g
LDFLAGS = $(LIBS)
