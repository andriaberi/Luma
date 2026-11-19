# Luma compiler (stage 0, written in C)
#
#   make          build build/luma
#   make clean    delete build/
#   make help     list targets

CC      ?= cc
CFLAGS   ?= -O0 -g3
WARN     := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes
CPPFLAGS += -Isrc

BIN := build/luma
SRC := $(shell find src -name '*.c')
OBJ := $(SRC:src/%.c=build/%.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# -MMD -MP: track header dependencies so editing a .h rebuilds what uses it.
build/%.o: src/%.c makefile
	@mkdir -p $(@D)
	$(CC) -std=c11 $(WARN) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf build

help:
	@sed -n 's/^#   //p' makefile

-include $(OBJ:.o=.d)

.PHONY: all clean help
