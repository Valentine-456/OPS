# Makefile for the hello program

# Compiler and compiler flags
CC = gcc
CFLAGS = -std=gnu99 -Wall -fsanitize=address,undefined 
LDLIBS = -lpthread -lm

# Target executable name
TARGET = concurrentArray.exe

# Source files
SRC = threadsAndSignals.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

format:
	clang-format -i ./*.c 
