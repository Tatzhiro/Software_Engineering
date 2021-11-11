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
#include "nonp-parallel-hash.hh"
#include <vector>
using namespace std;

//2,091,000
#define SZ_PAGE (1024 * 1024 * 100)
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))
#define PARTITION (sizeof(TUPLE) * SZ_PAGE / 1024)
//#define NB_BUFR (80000000)
//#define NB_BUFS (100000000)
//1024 * 100

// R = 1,000,000
// S = 2,000,000

// 4096
// 1024 * 8192

/*
typedef struct _TUPLE
{
    int key;
    int val;
} TUPLE;
*/

namespace psearch
{
    int thread_number = 4;
    int lock = 0;
};

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
    gettimeofday(&end, NULL);
    diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
    printf("Diff: %ld us (%ld ms)\n", diff, diff / 1000);
}

void join(hash_t *hashS, TUPLE bufR[NB_BUFR], myarg_t *arg, long int *sum, int *num_join)
{
    //printf("thread %d start\n", arg->thread_id);
    TUPLE *s_pointer = NULL;
    int resultVal = 0;
    int number_of_joins = 0;
    int expected;
    for (int j = 0; j < arg->marker_size; j++)
    {
        expected = 0;
        if (__atomic_compare_exchange_n(&arg->marker[j], &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        {
            for (int i = j * PARTITION; i < (j + 1) * PARTITION; i++)
            {
                s_pointer = Hash_Lookup(hashS, bufR[i].key);
                if (s_pointer != NULL)
                {
                    resultVal += bufR[i].val;
                    number_of_joins++;
                }
            }
        }
        else
        {
            usleep(1);
            continue;
        }
    }
    __atomic_add_fetch(num_join, number_of_joins, __ATOMIC_SEQ_CST);
    __atomic_add_fetch(sum, resultVal, __ATOMIC_SEQ_CST);
}

int main(void)
{
    int rfd;
    int sfd;
    int nr;
    int amount_of_nr = 0;
    int ns;
    int number_of_joins = 0;
    int count = 0;
    int marker_size;
    //TUPLE bufR[NB_BUFR];
    //TUPLE bufS[NB_BUFS];
    TUPLE *bufR = (TUPLE *)malloc(sizeof(TUPLE) * NB_BUFR);
    if (bufR == NULL)
        ERR;
    TUPLE *bufS = (TUPLE *)malloc(sizeof(TUPLE) * NB_BUFS);
    if (bufS == NULL)
        ERR;
    RESULT result;
    myarg_t args[psearch::thread_number];
    long int resultVal = 0;
    thread t[psearch::thread_number];
    struct timeval begin, end;

    rfd = open("R", O_RDONLY);
    if (rfd == -1)
        ERR;
    sfd = open("S", O_RDONLY);
    if (sfd == -1)
        ERR;
    hash_t *hashS = (hash_t *)malloc(sizeof(hash_t));
    if (hashS == NULL)
        ERR;

    gettimeofday(&begin, NULL);
    while (true)
    {
        cout << count++ << endl;
        nr = read(rfd, bufR, NB_BUFR * sizeof(TUPLE));
        amount_of_nr = nr / (int)sizeof(TUPLE);
        if (nr == -1)
            ERR;
        else if (nr == 0)
            break;

        if ((lseek(sfd, 0, SEEK_SET)) == -1)
            ERR;
        int marker[amount_of_nr / PARTITION];
        marker_size = sizeof(marker) / sizeof(marker[0]);
        initMarker(marker, marker_size);
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
            partitioningHashBuild(hashS, bufS, ns / (int)sizeof(TUPLE), PARTITION);
            //printDiff(begin, end);
            //hashBuild(hashS, bufS, ns / (int)sizeof(TUPLE));

            for (int i = 0; i < psearch::thread_number; i++)
            {
                args[i].marker = marker;
                args[i].marker_size = marker_size;
                args[i].thread_id = i + 1;
            }

            for (int i = 0; i < psearch::thread_number; i++)
            {
                //cout << "thread create" << endl;
                t[i] = thread(join, hashS, bufR, &args[i], &resultVal, &number_of_joins);
                //t.emplace_back(mythread, hash, p, args[i]);
            }
            for (int i = 0; i < psearch::thread_number; i++)
            {
                t[i].join();
            }

            /*
            TUPLE *s_pointer = NULL;
            for (int i = 0; i < nr / (int)sizeof(TUPLE); i++)
            {
                s_pointer = Hash_Lookup(hashS, bufR[i].key);
                if (s_pointer != NULL)
                {
                    //cout << "\tbufR[" << i << "].key = " << bufR[i].key << " bufS[" << s_pointer->key << "].key = " << s_pointer->key << endl;
                    result.rkey = bufR[i].key;
                    result.rval = bufR[i].val;
                    result.skey = s_pointer->key;
                    result.sval = s_pointer->val;
                    resultVal += result.rval;
                    number_of_joins++;
                }
            }
            */
        }
    }
    //countIndex(hashS);
    printDiff(begin, end);
    printf("Result: %ld\n", resultVal);
    printf("number of joins = %d\n", number_of_joins);
    return 0;
}

// R 100000 S 300000
// Diff: 2574397 us (2574 ms)
// Diff: 65313679 us (65313 ms)