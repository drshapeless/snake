include config.mk

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) vk_mem_alloc.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(filter-out main.o, $(OBJ)): %.o: %.h config.h

clean:
	rm -f $(BIN) $(OBJ)

run: $(BIN)
	./$(BIN)

.PHONY: all clean run
