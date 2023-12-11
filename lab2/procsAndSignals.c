#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                            \
  (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source),             \
   kill(0, SIGKILL), exit(EXIT_FAILURE))

#define UNUSED(x) (void)(x)

typedef struct {
  int issues;
  pid_t pid;
} Students;

volatile sig_atomic_t taskAccepted = 0;

void sethandler(void (*f)(int, siginfo_t *, void *), int sigNo) {
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_sigaction = f;
  act.sa_flags = SA_SIGINFO;

  if (-1 == sigaction(sigNo, &act, NULL))
    ERR("sigaction");
}

void parentSIGHandling(int sig, siginfo_t *siginfo, void *ucontext) {
  UNUSED(sig);
  UNUSED(ucontext);
  write(1, "Teacher has accepted solution of a student!\n", 45);
  if (kill(siginfo->si_pid, SIGUSR2))
    ERR("kill");
}

void childSIGHandling(int sig, siginfo_t *siginfo, void *ucontext) {
  UNUSED(sig);
  UNUSED(siginfo);
  UNUSED(ucontext);
  taskAccepted = sig;
}

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

int child_work(int p, int t, int probability) {
  srand(time(NULL) * getpid());
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

    while (taskAccepted != SIGUSR2) {
      kill(getppid(), SIGUSR1);
      millisleep(10);
    }
    taskAccepted = 0;
  }
  printf("Student [%d] has finished the task, having %d issues!\n", getpid(),
         issues);
  return issues;
}

void create_children(int argc, char *argv[], Students data[]) {
  int p = atoi(argv[1]);
  int t = atoi(argv[2]);
  for (int i = 3; i < argc; i++) {
    int pid;
    switch (pid = fork()) {
    case 0:
      sethandler(childSIGHandling, SIGUSR2);
      int issuesCounter = child_work(p, t, atoi(argv[i]));
      exit(issuesCounter);
    case -1:
      perror("Fork:");
      exit(EXIT_FAILURE);
    default:
      data[i - 3].pid = pid;
      data[i - 3].issues = 0;
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
  int n = argc - 3;
  Students *students = malloc(sizeof(Students) * n);

  create_children(argc, argv, students);
  sethandler(parentSIGHandling, SIGUSR1);

  while (n > 0) {
    // millisleep(1000);
    int issuesExitStatus;
    pid_t pid;
    for (;;) {
      pid = waitpid(0, &issuesExitStatus, WNOHANG);
      if (pid > 0) {
        for (int i = 0; i < argc - 3; i++) {
          if (students[i].pid == pid) {
            students[i].issues += WEXITSTATUS(issuesExitStatus);
            break;
          }
        }
        n--;
      }
      if (0 == pid)
        break;
      if (0 >= pid) {
        if (ECHILD == errno)
          break;
        ERR("waitpid:");
      }
    }
  }

  printf("Number |  ID   | Issues\n");
  for (int i = 0; i < argc - 3; i++) {
    printf("%6d | %5d | %5d\n", i + 1, students[i].pid, students[i].issues);
  }
  free(students);

  return 0;
}