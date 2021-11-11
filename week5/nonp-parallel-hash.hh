#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <vector>
#include "../debug.h"
#include "../integers.hh"

using namespace std;

#define BUCKETS (10000000)
//#define NB_BUFR (5000000)
//#define NB_BUFS (10000000)
int thread_number = 4;

typedef struct _TUPLE
{
    int key;
    int val;
} TUPLE;

typedef struct __node_t
{
    TUPLE *key;
    struct __node_t *next;
} node_t;

typedef struct __list_t
{
    node_t *head;
    //pthread_mutex_t lock;
    int lock;
} list_t;

typedef struct __hash_t
{
    list_t lists[BUCKETS];
} hash_t;

typedef struct
{
    int thread_id;
    int start;
    int end;
    int *marker;
    int marker_size;
} myarg_t;

void List_Init(list_t *L)
{
    L->head = NULL;
    //pthread_mutex_init(&L->lock, NULL);
    L->lock = 0;
}

bool myLock(int &flag)
{
    int expected = 0;
    return __atomic_compare_exchange_n(&flag, &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void unLock(int &flag)
{
    __atomic_store_n(&flag, 0, __ATOMIC_SEQ_CST);
}

bool List_Insert(list_t *L, TUPLE *key)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    if (node == NULL)
        ERR;
    node->key = key;
    while (1)
    {
        if (myLock(L->lock))
        {
            node->next = L->head;
            L->head = node;
            unLock(L->lock);
            return 1;
        }
        else
        {
            usleep(1);
        }
    }
}

TUPLE *List_Lookup(list_t *L, int id)
{
    myLock(L->lock);
    node_t *curr = L->head;
    while (curr)
    {
        if (curr->key->key == id)
        {
            unLock(L->lock);
            return curr->key; // success
        }
        curr = curr->next;
    }
    unLock(L->lock);
    return NULL; // failure
}

void Hash_Init(hash_t *H)
{
    for (int i = 0; i < BUCKETS; i++)
        List_Init(&H->lists[i]);
}

bool Hash_Insert(hash_t *H, TUPLE *key)
{
    return List_Insert(&H->lists[key->key % BUCKETS], key);
}

TUPLE *Hash_Lookup(hash_t *H, int id)
{
    return List_Lookup(&H->lists[id % BUCKETS], id);
}

void printer(hash_t *hash)
{
    node_t *n;
    printf("Bucket\n");
    for (int i = 0; i < 2; i++)
    {
        printf("[%d] ", i);
        if (hash->lists[i].head == NULL)
        {
            printf("\n");
            continue;
        }
        n = hash->lists[i].head;
        while (n != NULL)
        {
            printf("%d ", n->key->key);
            n = n->next;
        }
        printf("\n");
    }
}

void mythread(hash_t *hash, TUPLE *p, myarg_t *args, int partition)
{
    int expected;
    for (int j = 0; j < args->marker_size; j++)
    {
        expected = 0;
        if (__atomic_compare_exchange_n(&args->marker[j], &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        {
            for (int i = j * partition; i < (j+1) * partition; i++)
            {
                //cout << "thread " << args->thread_id << ": " << i << endl;
                Hash_Insert(hash, &(*(p + i)));
            }
        }
        else
        {
            usleep(1);
            continue;
        }
    }
    return;
}

void countIndex(hash_t *hash)
{
    node_t *n = NULL;
    int size_of_index;
    int hash_size = 0;
    for (int i = 0; i < BUCKETS; i++)
    {
        n = hash->lists[i].head;
        size_of_index = 0;
        while (n != NULL)
        {
            n = n->next;
            size_of_index++;
        }
        printf("|[%d]| %d\n", i, size_of_index);
        hash_size += size_of_index;
    }
    printf("size of hash = %d\n", hash_size);
}

void initMarker(int marker[], int size)
{
    for (int i = 0; i < size; i++)
        marker[i] = 0;
}

hash_t *partitioningHashBuild(hash_t *hash, TUPLE *p, int length, int partition)
{
    /*
    hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
    if (hash == NULL)
        ERR;
    */
    //while (thread_number < 10)
    //{

    Hash_Init(hash);
    thread t[thread_number];
    myarg_t args[thread_number];
    int marker[length / partition];
    int marker_size = sizeof(marker) / sizeof(marker[0]);
    initMarker(marker, marker_size);
    for (int i = 0; i < thread_number; i++)
    {
        args[i].marker = marker;
        args[i].marker_size = marker_size;
        args[i].thread_id = i + 1;
    }

    for (int i = 0; i < thread_number; i++)
    {
        //cout << "thread create" << endl;
        t[i] = thread(mythread, hash, p, &args[i], partition);
        //t.emplace_back(mythread, hash, p, args[i]);
    }
    for (int i = 0; i < thread_number; i++)
    {
        t[i].join();
    }
    //    thread_number++;
    //}
    return hash;
}

hash_t *hashBuild(hash_t *hash, TUPLE *p, int length)
{
    Hash_Init(hash);
    for (int i = 0; i < length; i++)
    {
        Hash_Insert(hash, &(*(p + i)));
    }
    return hash;
}