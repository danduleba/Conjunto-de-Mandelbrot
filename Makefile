CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2 -fopenmp -pthread
TARGET = mandelbrot

all: $(TARGET)

$(TARGET): mandelbrot.c
	$(CC) $(CFLAGS) mandelbrot.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean