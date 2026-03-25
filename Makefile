CC = gcc
CPPFLAGS = -Isrc -Isrc/graphics -Isrc/graphics/map -Isrc/genetic
CFLAGS = -Wall -ansi
LDLIBS = -lMLV -lm

TARGET = build/graphic

SRC = \
	src/graphics/graphic.c \
	src/graphics/map/map.c

OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build

.PHONY: all run clean