#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)                                                            \
  (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),             \
   exit(EXIT_FAILURE))

typedef struct {
  int *array;
  int arraySize;
  pthread_mutex_t *mainMXptr;
} PrintThreadParams;

volatile sig_atomic_t swapSignal = 0;
volatile sig_atomic_t printSignal = 0;

void sethandler(void (*f)(int), int sigNo) {
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = f;
  act.sa_flags = SA_SIGINFO;

  if (-1 == sigaction(sigNo, &act, NULL))
    ERR("sigaction");
}

void sigusr1Handler(int sig) { swapSignal = sig; }
void sigusr2Handler(int sig) { printSignal = sig; }

void ReadArguments(int argc, char **argv, int *n, int *p) {
  if (argc == 3) {
    *n = atoi(argv[1]);
    *p = atoi(argv[2]);
    if ((*n <= 8) || (*n >= 256)) {
      printf("Invalid value for 'n'\n");
      exit(EXIT_FAILURE);
    }
    if ((*p <= 1) || (*p >= 16)) {
      printf("Invalid value for 'p'\n");
      exit(EXIT_FAILURE);
    }
  } else {
    printf("Wrong number of params");
    exit(EXIT_FAILURE);
  }
}

void swap(int *num1, int *num2) {
  int temp = *num1;
  *num1 = *num2;
  *num2 = temp;
}

void millisleep(int millis) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = millis * 1000000;
  nanosleep(&time, NULL);
}

void SwapRoutine(int *array, pthread_mutex_t *arrayMx,
                 pthread_mutex_t *mainMXptr, int n) {
  srand(time(NULL));
  swapSignal = 0;
  int a = rand() % n;
  int b = rand() % n;
  while (a == b) {
    b = rand() % n;
  }
  if (a > b)
    swap(&a, &b);
  printf("a: %d, b: %d\n", a, b);

  pthread_mutex_lock(mainMXptr);
  for (int i = 0; i < floor((b - a - 1) / 2); i++) {
    pthread_mutex_lock(&arrayMx[a + i]);
    pthread_mutex_lock(&arrayMx[b - i]);
    swap(&array[a + i], &array[b - i]);
    printf("Swaped %d and %d\n", array[a + i], array[b - i]);
    millisleep(5);
    pthread_mutex_unlock(&arrayMx[a + i]);
    pthread_mutex_unlock(&arrayMx[b - i]);
  }
  pthread_mutex_unlock(mainMXptr);
}

void *PrintRoutine(void *voidParam) {
  PrintThreadParams *params = (PrintThreadParams *)voidParam;
  pthread_mutex_lock(params->mainMXptr);
  printf("Array:\n");
  for (int i = 0; i < params->arraySize; i++) {
    printf("%d\n", params->array[i]);
  }
  pthread_mutex_unlock(params->mainMXptr);
  return;
}

int main(int argc, char **argv) {
  int n, p;
  ReadArguments(argc, argv, &n, &p);
  int *array = (int *)malloc(sizeof(int) * n);
  pthread_mutex_t *arrayMx =
      (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * n);
  pthread_mutex_t *mainMXptr =
      (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_t mainMX = *mainMXptr;
  pthread_mutex_init(mainMXptr, NULL);

  for (int i = 0; i < n; i++) {
    array[i] = i;
    pthread_mutex_init(&(arrayMx[i]), NULL);
  }

  sethandler(sigusr1Handler, SIGUSR1);
  sethandler(sigusr2Handler, SIGUSR2);
  while (true) {
    millisleep(10);
    if (printSignal == SIGUSR2) {
      pthread_t tid;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      PrintThreadParams *param =
          (PrintThreadParams *)malloc(sizeof(PrintThreadParams));
      param->array = array;
      param->mainMXptr = &mainMX;
      param->arraySize = n;
      pthread_create(&tid, &attr, PrintRoutine, param);
      printSignal = 0;
    }
    if (swapSignal == SIGUSR1) {
      SwapRoutine(array, arrayMx, &mainMX, n);
    }
  }

  free(arrayMx);
  free(array);
  return 0;
}
