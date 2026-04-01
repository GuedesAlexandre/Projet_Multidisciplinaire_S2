CC := gcc
CFLAGS := -Wall -ansi
LDFLAGS := -lm

TARGET := app
SRCDIR := src
OBJDIR := obj

SRC := \
	$(SRCDIR)/main.c \
	$(SRCDIR)/data/municipality.c \
	$(SRCDIR)/parsers/parser_csv.c \
	$(SRCDIR)/genetic/generation.c \
	$(SRCDIR)/genetic/fitness.c \
	$(SRCDIR)/genetic/selection.c \
	$(SRCDIR)/genetic/mutation.c \
	$(SRCDIR)/genetic/crossover.c \
	$(SRCDIR)/graphics/graphic.c \
	$(SRCDIR)/graphics/map/map.c

OBJ := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean