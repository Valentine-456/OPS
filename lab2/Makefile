# Makefile for the hello program

# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# Target executable name
TARGET = labSimulator.exe

# Source files
SRC = procsAndSignals.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

format:
	clang-format -i ./*.c 
