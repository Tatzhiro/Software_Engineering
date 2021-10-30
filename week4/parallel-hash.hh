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

#define BUCKETS (100000)
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
    pthread_mutex_t lock;
    //int lock;
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
} myarg_t;

void List_Init(list_t *L)
{
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
    //L->lock = 0;
}

/*
bool myLock(int flag)
{
    int expected = 0;
    return __atomic_compare_exchange_n(&flag, &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void unLock(int flag)
{
    __atomic_store_n(&flag, 0, __ATOMIC_SEQ_CST);
}
*/

bool List_Insert(list_t *L, TUPLE *key)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    if (node == NULL)
        ERR;
    node->key = key;
    while (1)
    {
        if (pthread_mutex_trylock(&L->lock) == 0)
        {
            node->next = L->head;
            L->head = node;
            pthread_mutex_unlock(&L->lock);
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
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    while (curr)
    {
        if (curr->key->key == id)
        {
            pthread_mutex_unlock(&L->lock);
            return curr->key; // success
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
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
    for (int i = 0; i < BUCKETS; i++)
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

void mythread(hash_t *hash, TUPLE *p, myarg_t *args)
{
    for (int i = args->start; i < args->end; i++)
    {
        //cout << "thread " << args->thread_id << ": " << i << endl;
        Hash_Insert(hash, &(*(p + i)));
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

hash_t *create_hash_table(hash_t *hash, TUPLE *p, int length)
{
    /*
    hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
    if (hash == NULL)
        ERR;
    */
    //while (thread_number < 10)
    //{

    Hash_Init(hash);
    thread t[thread_number - 1];
    myarg_t args[thread_number - 1];
    for (int i = 0; i < thread_number - 1; i++)
    {
        args[i].start = length / thread_number * i;
        args[i].end = length / thread_number * (i + 1);
        args[i].thread_id = i + 1;
    }

    for (int i = 0; i < thread_number - 1; i++)
    {
        //cout << "thread create" << endl;
        t[i] = thread(mythread, hash, p, &args[i]);
        //t.emplace_back(mythread, hash, p, args[i]);
    }

    for (int i = length / thread_number * (thread_number - 1); i < length; i++)
    {
        //cout << "main " << i << endl;
        Hash_Insert(hash, &(*(p + i)));
    }

    for (int i = 0; i < thread_number - 1; ++i)
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