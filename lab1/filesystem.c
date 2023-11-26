#define _GNU_SOURCE
#include "./textMessages.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
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

ssize_t bulk_read(int fd, char *buf, size_t count) {
  ssize_t c;
  ssize_t len = 0;
  do {
    c = TEMP_FAILURE_RETRY(read(fd, buf, count));
    if (c < 0)
      return c;
    if (c == 0)
      return len; // EOF
    buf += c;
    len += c;
    count -= c;
  } while (count > 0);
  return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count) {
  ssize_t c;
  ssize_t len = 0;
  do {
    c = TEMP_FAILURE_RETRY(write(fd, buf, count));
    if (c < 0)
      return c;
    buf += c;
    len += c;
    count -= c;
  } while (count > 0);
  return len;
}

void showDir(char *filepath) {
  DIR *dir;
  struct dirent *dp;
  struct stat filestat;
  char currentDir[256];

  if (NULL == (dir = opendir(filepath)))
    ERR("opendir");
  printf("Ok, there are following files in this directory:\n");
  getcwd(currentDir, sizeof(currentDir));
  chdir(filepath);

  do {
    errno = 0;
    if ((dp = readdir(dir)) != NULL) {
      if (lstat(dp->d_name, &filestat))
        ERR("lstat");
      printf("-  %s \n", dp->d_name);
    }
  } while (dp != NULL);

  chdir(currentDir);
  closedir(dir);
}

void showFile(char *filepath) {
  const int fd = open(filepath, O_RDONLY);
  if (fd == -1)
    ERR("open");
  char file_buf[256];
  struct stat filestat;

  for (;;) {
    const ssize_t read_size = bulk_read(fd, file_buf, 256);
    if (read_size == -1)
      ERR("bulk_read");

    if (read_size == 0)
      break;

    if (bulk_write(1, file_buf, read_size) == -1)
      ERR("bulk_write");
  }

  if (close(fd) == -1)
    ERR("close");

  lstat(filepath, &filestat);
  printf("\nSize: %ld bytes \n", filestat.st_size);
  printf("Group ID: %d\n", filestat.st_gid);
  printf("User ID: %d\n", filestat.st_uid);
}

display_info(const char *fpath, const struct stat *sb, int tflag,
             struct FTW *ftwbuf) {
  for (int i = 0; i < ftwbuf->level; i++)
    printf(" ");
  if (S_ISDIR(sb->st_mode))
    printf("+");
  else
    printf(" ");

  printf("%s\n", basename(fpath));
  return 0;
}

int walk_stage4(char *filepath) {
  int flags = 0;
  flags |= FTW_PHYS;

  if (nftw(filepath, display_info, 20, flags) == -1) {
    perror("nftw");
    exit(EXIT_FAILURE);
  }
}

int show_stage3(char *filepath) {
  struct stat buffer;
  stat(filepath, &buffer);

  if (S_ISDIR(buffer.st_mode)) {
    showDir(filepath);
  } else if (S_ISREG(buffer.st_mode)) {
    showFile(filepath);
  } else {
    showError("Type of the file is unknown");
  }

  return 0;
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
    if (getFilepath(filepath))
      show_stage3(filepath);
    else
      showError("File doesn't exist");
    break;
  case 'c':
    if (getFilepath(filepath))
      walk_stage4(filepath);
    else
      showError("File doesn't exist");
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
