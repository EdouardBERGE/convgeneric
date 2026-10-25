CC ?= gcc
CFLAGS ?= -O2 -Wall
LDLIBS ?= -lpng -lz -lm

TARGET = convgeneric
SRCS = convgeneric.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS) palette.h commandLine.c hardware_sprite.c split_raster.c screen_sprite.c palette_processing.c
	$(CC) $(CFLAGS) -o $@ convgeneric.c $(LDLIBS)

clean:
	rm -f $(TARGET)

test:
	./tests/run.sh
