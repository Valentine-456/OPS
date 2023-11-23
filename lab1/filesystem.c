#pragma once
#include "./textMessages.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 999
#define ERR(message)                                                           \
  (perror(message), fprintf(stderr, message), exit(EXIT_FAILURE))

bool fileExists(char *path) {
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

bool isRegularFile(char *path) {
  struct stat buffer;
  stat(path, &buffer);
  return S_ISREG(buffer.st_mode);
}

bool getFilepath(char *path) {
  printf("Ok, enter the file path:\n");

  fgets(path, MAX_STRING_LENGTH * sizeof(char), stdin);
  size_t len = strlen(path);
  if (len > 0 && path[len - 1] == '\n') {
    path[len - 1] = '\0';
  }

  return fileExists(path);
}

int write_stage2(char *filepath) {
  printf("Ok, enter information you want to write to the file:\n> ");
  char lastChar;
  char currentChar = '\t';

  if (!isRegularFile(filepath)) {
    showError("Provided file is not a regular file");
    return 1;
  }

  FILE *file;
  if ((file = fopen(filepath, "w+")) != NULL) {
    remove(filepath);
    fclose(file);
  }
  file = fopen(filepath, "ab+");

  while (true) {
    lastChar = currentChar;
    currentChar = getchar();
    if ((currentChar == '\n') && (lastChar == '\n')) {
      fflush(file);
      fclose(file);
      printf("The End of file writing\n");
      break;
    }
    fputc(toupper(currentChar), file);
  }

  return 0;
}

int interface_stage1() {
  char command;
  char buff[MAX_STRING_LENGTH];
  char filepath[MAX_STRING_LENGTH];

  printf("\nWelcome to CLI filemanager! Choose your command:\n");
  showOptions();
  fgets(buff, MAX_STRING_LENGTH * sizeof(char), stdin);
  command = tolower(buff[0]);

  switch (command) {
  case 'a':
    if (getFilepath(filepath))
      write_stage2(filepath);
    else
      showError("File doesn't exist");
    break;
  case 'b':
    printf("show\n");
    break;
  case 'c':
    printf("walk\n");
    break;
  case 'd':
    return 0;
    break;
  default:
    showError("Unknown command");
    return -1;
    break;
  }
  return 1;
}

int main() {
  while (interface_stage1() != 0)
    ;
  return 0;
}
