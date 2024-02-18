include config.mk

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)


clean:
	rm -f $(BIN) $(OBJ)

.PHONY: all clean
