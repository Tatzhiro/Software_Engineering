#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "../debug.h"
#include <iostream>
using namespace std;

#define SZ_PAGE 4096
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))

// 1024 * 8192
// block-nested-loop-join

typedef struct _TUPLE
{
  int key;
  int val;
} TUPLE;

typedef struct _RESULT
{
  int rkey;
  int rval;
  int skey;
  int sval;
} RESULT;

void printDiff(struct timeval begin, struct timeval end)
{
  long diff;

  diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  printf("Diff: %ld us (%ld ms)\n", diff, diff / 1000);
}

int main(void)
{
  int rfd;
  int sfd;
  int nr;
  int ns;
  int count = 0;
  int number_of_joins = 0;
  TUPLE bufR[NB_BUFR];
  TUPLE bufS[NB_BUFS];
  RESULT result;
  int resultVal = 0;
  struct timeval begin, end;

  rfd = open("R", O_RDONLY);
  if (rfd == -1)
    ERR;
  sfd = open("S", O_RDONLY);
  if (sfd == -1)
    ERR;

  gettimeofday(&begin, NULL);
  while (true)
  {
    cout << count++ << endl;
    nr = read(rfd, bufR, NB_BUFR * sizeof(TUPLE));
    if (nr == -1)
      ERR;
    else if (nr == 0)
      break;

    if ((lseek(sfd, 0, SEEK_SET)) == -1)
      ERR;
    while (true)
    {
      ns = read(sfd, bufS, NB_BUFS * sizeof(TUPLE));
      if (ns == -1)
        ERR;
      else if (ns == 0)
        break;

      //cout << "nr / (int)sizeof(TUPLE) = " << nr / (int)sizeof(TUPLE) << endl;
      //cout << "ns / (int)sizeof(TUPLE) = " << ns / (int)sizeof(TUPLE) << endl;
      // join
      for (int i = 0; i < nr / (int)sizeof(TUPLE); i++)
      {
        for (int j = 0; j < ns / (int)sizeof(TUPLE); j++)
        {
          if (bufR[i].key == bufS[j].key)
          {
            //cout << "i = " << i << endl;
            //cout << "\tbufR[" << i << "].key = " << bufR[i].key << " bufS[" << j << "].key = " << bufS[j].key << endl;
            result.rkey = bufR[i].key;
            result.rval = bufR[i].val;
            result.skey = bufS[j].key;
            result.sval = bufS[j].val;
            resultVal += result.rval;
            number_of_joins++;
          }
        }
      }
    }
  }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  printf("Result: %d\n", resultVal);
  printf("number of joins = %d\n", number_of_joins);
  return 0;
}
