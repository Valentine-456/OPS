#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                            \
  (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source),             \
   kill(0, SIGKILL), exit(EXIT_FAILURE))

void child_work(int p, int t, int probability) {
  printf("Student [%d] with probability %d has started doing task!\n", getpid(),
         probability);
}

void create_children(int argc, char *argv[]) {
  int p = atoi(argv[1]);
  int t = atoi(argv[2]);
  for (int i = 3; i < argc; i++) {
    switch (fork()) {
    case 0:
      child_work(p, t, atoi(argv[i]));
      exit(EXIT_SUCCESS);
    case -1:
      perror("Fork:");
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  int p, t;
  if (argc < 4) {
    fprintf(stderr, "Program runs with at least 3 parameters\n");
    return 1;
  }
  p = atoi(argv[1]);
  t = atoi(argv[2]);
  bool timeParamsWrong = (1 > p) || (10 < p) || (1 >= t) || (10 < t);
  if (timeParamsWrong) {
    fprintf(stderr, "Parameters range should be: 1 <= p <= 10; 1 < t <= 10\n");
    return 1;
  }

  for (int i = 3; i < argc; i++) {
    if ((atoi(argv[i]) > 100) || (atoi(argv[i]) < 0)) {
      fprintf(stderr, "Probability params range should be: 0 <= prob <= 100\n");
      return 1;
    }
  }

  create_children(argc, argv);
  while (wait(NULL) > 0)
    ;
  return 0;
}