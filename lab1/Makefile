# Makefile for the hello program

# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# Target executable name
TARGET = caecilius.exe

# Source files
SRC = filesystem.c textMessages.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

format:
	clang-format -i ./*.c 
