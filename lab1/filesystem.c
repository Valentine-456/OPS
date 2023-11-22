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

bool getFilepath(char *path) {
  printf("Ok, enter the file path:\n");

  fgets(path, MAX_STRING_LENGTH * sizeof(char), stdin);
  size_t len = strlen(path);
  if (len > 0 && path[len - 1] == '\n') {
    path[len - 1] = '\0';
  }

  return fileExists(path);
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
      printf("stage2\n");
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
