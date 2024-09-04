BIN = snake

# compiler
CC = clang
CXX = clang++

# includes and libs
INCS = $(shell pkg-config --cflags sdl3)
LIBS = $(shell pkg-config --libs sdl3) -lvulkan -lstdc++

# flags
CFLAGS = $(INCS) -O2 -std=c18 -g
CXXFLAGS = $(INCS) -O2 -std=c++14 -Wno-nullability-completeness
LDFLAGS = $(LIBS)
