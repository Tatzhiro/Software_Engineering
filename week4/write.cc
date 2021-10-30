#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>
#include "../debug.h"

typedef struct _TUPLE
{
  int key;
  int val;
} TUPLE;

#define FILE "DATA"

void writeToStorage(const int max, char *file)
{
  int fd;
  TUPLE t;
  int key = 0;
  srand(time(NULL));

  fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if (fd == -1)
    ERR;
  for (int i = 0; i < max; i++)
  {
    t.key = key++;
    t.val = rand() % 100;
    write(fd, &t, sizeof(t));
  }
  close(fd);
}

void readFromStorafe(char *file)
{
  int fd;
  int ret;
  TUPLE t;
  fd = open(file, O_RDONLY);
  if (fd == -1)
    ERR;
  while (1)
  {
    ret = read(fd, &t, sizeof(t));
    if (ret == 0)
      break;
    else if (ret == -1)
      ERR;
    printf("%d\t%d\n", t.key, t.val);
  }
  close(fd);
}

int main(int argc, char *argv[]) // argv[1] = int max, argv[2] = FILE
{
  int max;

  /*if (argc != 2)
    max = 10;
  else*/
  max = atoi(argv[1]);
  std::cout << argv[2] << std::endl;
  writeToStorage(max, argv[2]);
  readFromStorafe(argv[2]);

  return 0;
}
