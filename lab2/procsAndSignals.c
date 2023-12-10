#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <asm-generic/signal-defs.h>

#define ERR(source)                                                            \
  (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source),             \
   kill(0, SIGKILL), exit(EXIT_FAILURE))

// void sethandler(void (*f)(int), int sigNo) {
//   struct sigaction act;
//   memset(&act, 0, sizeof(struct sigaction));
//   act.sa_handler = f;
//   if (-1 == sigaction(sigNo, &act, NULL))
//     ERR("sigaction");
// }

// parentSIGHandling(int param) {}

void millisleep(int millis) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = millis * 1000000;
  nanosleep(&time, NULL);
}

int randBinaryVariable(int chance) {
  int result = (rand() % (100 - 0 + 1)) + 0;
  return result < chance;
}

void child_work(int p, int t, int probability) {
  int issues = 0;
  printf("Student [%d] with probability %d has started doing task!\n", getpid(),
         probability);

  for (int i = 0; i < p; i++) {
    printf("Student [%d] started doing part %d of %d!\n", getpid(), i + 1, p);
    for (int j = 0; j < t; j++) {
      millisleep(100);
      if (randBinaryVariable(probability)) {
        printf("Student [%d] has issue (%d) doing task\n", getpid(), ++issues);
        millisleep(50);
      }
    }
    printf("Student [%d] has finished doing part %d of %d!\n", getpid(), i + 1,
           p);
    kill(getppid(), SIGUSR1);
  }
  printf("Student [%d] has finished the task, having %d issues!\n", getpid(),
         issues);
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

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  create_children(argc, argv);
  while (wait(NULL) > 0)
    ;
  return 0;
}