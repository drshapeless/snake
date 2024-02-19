include config.mk

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

wasm: CC=emcc
wasm: CFLAGS += -sMALLOC=emmalloc
wasm: $(OBJ)
	$(CC) -o $(BIN).html -sGL_ENABLE_GET_PROC_ADDRESS $(OBJ) libSDL3.a --shell-file template.html

wasm_debug: CC=emcc
wasm_debug: CFLAGS += -fsanitize=undefined
wasm_debug: $(OBJ)
	$(CC) -o $(BIN).html -sGL_ENABLE_GET_PROC_ADDRESS -s $(OBJ) libSDL3.a -s ASSERTIONS=2 -fsanitize=undefined

clean:
	rm -f $(BIN) $(OBJ) $(BIN).*

.PHONY: all clean
