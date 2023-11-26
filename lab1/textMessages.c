#include <stdio.h>

void showOptions() { printf("A. write\nB. show\nC. walk\nD. exit\n"); }
void showError(char *message) { fprintf(stderr, "Error: %s\n", message); }