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
  pthread_mutex_t *arrayMx;
} PrintThreadParams;

typedef struct {
  int *array;
  int arraySize;
  pthread_mutex_t *arrayMx;
  int *threadCurrentStatus;
} SwapThreadParams;

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

void sigusr1Handler(int sig) { swapSignal += 1; }
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

void *SwapRoutine(void *voidParam) {
  SwapThreadParams *params = (SwapThreadParams *)voidParam;
  srand(time(NULL));
  int a = rand() % params->arraySize;
  int b = rand() % params->arraySize;
  while (a == b) {
    b = rand() % params->arraySize;
  }
  if (a > b)
    swap(&a, &b);
  printf("a: %d, b: %d\n", a, b);

  for (int i = 0; i < floor((b - a - 1) / 2); i++) {
    pthread_mutex_lock(&params->arrayMx[a + i]);
    pthread_mutex_lock(&params->arrayMx[b - i]);
    swap(&params->array[a + i], &params->array[b - i]);
    printf("Swaped %d and %d\n", params->array[a + i], params->array[b - i]);
    millisleep(50);
    pthread_mutex_unlock(&params->arrayMx[a + i]);
    pthread_mutex_unlock(&params->arrayMx[b - i]);
  }
  *params->threadCurrentStatus = 0;
  free(params);
  return;
}

void *PrintRoutine(void *voidParam) {
  PrintThreadParams *params = (PrintThreadParams *)voidParam;
  for (int i = 0; i < params->arraySize; i++) {
    pthread_mutex_lock(&params->arrayMx[i]);
  }
  printf("Array:\n");
  for (int i = 0; i < params->arraySize; i++) {
    printf("%d\n", params->array[i]);
  }
  for (int i = 0; i < params->arraySize; i++) {
    pthread_mutex_unlock(&params->arrayMx[i]);
  }
  free(params);
  return;
}

int main(int argc, char **argv) {
  int n, p;
  ReadArguments(argc, argv, &n, &p);
  int *array = (int *)malloc(sizeof(int) * n);
  pthread_mutex_t *arrayMx =
      (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * n);
  for (int i = 0; i < n; i++) {
    array[i] = i;
    pthread_mutex_init(&(arrayMx[i]), NULL);
  }
  int *threadsStatuses = (int *)malloc(sizeof(int) * p);
  for (int i = 0; i < p; i++) {
    threadsStatuses[i] = 0;
  }

  sethandler(sigusr1Handler, SIGUSR1);
  sethandler(sigusr2Handler, SIGUSR2);
  while (true) {
    if (printSignal == SIGUSR2) {
      pthread_t tid;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      PrintThreadParams *param =
          (PrintThreadParams *)malloc(sizeof(PrintThreadParams));
      param->array = array;
      param->arrayMx = arrayMx;
      param->arraySize = n;
      pthread_create(&tid, &attr, PrintRoutine, param);
      printSignal = 0;
    }
    if (swapSignal > 0) {
      bool hasVacantThread = false;
      int *vacantThreadStatusPtr;
      for (int i = 0; i < p; i++) {
        if (threadsStatuses[i] == 0) {
          threadsStatuses[i] = 1;
          vacantThreadStatusPtr = &threadsStatuses[i];
          hasVacantThread = true;
          break;
        }
      }
      if (hasVacantThread) {
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        SwapThreadParams *param =
            (SwapThreadParams *)malloc(sizeof(SwapThreadParams));
        param->array = array;
        param->arrayMx = arrayMx;
        param->arraySize = n;
        param->threadCurrentStatus = vacantThreadStatusPtr;
        pthread_create(&tid, &attr, SwapRoutine, param);
      } else {
        printf("All threads are busy right now\n");
      }
      swapSignal--;
    }
  }

  free(arrayMx);
  free(array);
  free(threadsStatuses);
  return 0;
}
