BIN = minekraft.elf

LIB_FOLDER = lib

SRC_DIR = src

CC = gcc-9
CFLAGS = -Wall -Wextra -O0 -g -I$(LIB_FOLDER)/include/ -I$(SRC_DIR)/include/ $(shell pkg-config --cflags glfw3 gl)
LDFLAGS = $(shell pkg-config --libs gl glew glfw3) -pthread -lm

BUILD = build

INTERNALCFLAGS  :=       \
	-std=gnu17           \

SRCDIRS := $(shell find ./$(LIB_FOLDER) -type d)
SRCDIRS += $(shell find ./$(SRC_DIR) -type d)

CFILES := $(shell find ./$(LIB_FOLDER) -type f -name '*.c')
CFILES += $(shell find ./$(SRC_DIR) -type f -name '*.c')

OBJ    := $(CFILES:%.c=$(BUILD)/%.o)

$(shell mkdir -p $(addprefix $(BUILD)/,$(SRCDIRS)))

all: NODEBUG

.PHONY: all clean NODEBUG clean_deps

NODEBUG: $(BIN)
	@./$<

$(BIN): $(OBJ)
	@$(CC) $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.c
	@$(CC) -o $@ $(CFLAGS) $(INTERNALCFLAGS) -c $<

$(BUILD)/%.o: %.s
	@nasm $(ASMPARAM) -o $@ $<

clean:
	@rm -rf $(BUILD) $(BIN)
