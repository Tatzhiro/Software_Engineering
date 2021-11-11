#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define BUCKET_SIZE 10
#define length 1000000

#define lower 1
#define upper 1000 * length

// (upper - lower) has to be greater than length

typedef struct chain
{
    int data;
    struct chain *next;
} CHAIN;

typedef struct
{
    CHAIN *head;
    int thread_id;
    int start;
    int end;
} myarg_t;

int chain_size[BUCKET_SIZE];
void create_hash_table(int *p);
void printer(CHAIN **hash);

int thread_number = 5;
int key = 0;
int indx = 0;
CHAIN *hash[BUCKET_SIZE];

int search(int arr[], int n, int x, int copy)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (arr[i] == x)
        {
            arr[i] = copy;
            return 0;
        }
    }
    printf("search failed\n");
    abort();
}

int insertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
    return j + 1;
}

int binarysearch(int *x, int goal, int tail)
{
    int head = 0;
    tail = tail - 1;
    while (1)
    {
        int search_key = floor((head + tail) / 2);
        if (x[search_key] == goal)
        {
            return 1;
        }
        else if (goal > x[search_key])
        {
            head = search_key + 1;
        }
        else if (goal < x[search_key])
        {
            tail = search_key - 1;
        }
        if (tail < head)
        {
            return -1;
        }
    }
}

int cmp(const int *a, const int *b)
{
    return *a < *b ? -1 : *a > *b ? 1
                                  : 0;
}

int *unique_random_integer(void)
{
    int r = 0;
    int flag = 0;
    int *p = calloc(length, sizeof(int));
    int *cp = calloc(length, sizeof(int));
    if (p == NULL || cp == NULL)
    {
        printf("failed to allocate memory\n");
        abort();
    }
    else
    {
        srand(time(NULL));
        for (int i = 0; i < length; i++)
        {
            //r = rand();
            r = (rand() % (upper - lower + 1)) + lower;
            *(p + i) = r;
            *(cp + i) = r;
        }
        qsort(p, length, sizeof(int), (int (*)(const void *, const void *))cmp);
        for (int i = 1; i < length; i++)
        {
            if (*(p + i - 1) == *(p + i))
            {
                while (1)
                {
                    //r = rand();
                    r = (rand() % (upper - lower + 1)) + lower;
                    flag = binarysearch(p, r, length);
                    if (flag == -1)
                    {
                        search(cp, length, *(p + i), r);
                        *(p + i) = r;
                        break;
                    }
                }
                insertionSort(p, length);
                i--;
            }
        }
        free(p);
    }
    return cp;
}

void hashSearch(CHAIN *head, int key)
{
    int i = 0;
    for (i = 0; i < chain_size[indx] / thread_number * (thread_number - 1); i++)
    {
        head = head->next;
    }
    while (head != NULL)
    {
        if (head->data == key)
        {
            printf("Main: The index of %d is [%d][%d]\n", key, indx, i);
            return;
        }
        head = head->next;
        i++;
    }
    printf("Main:      %d does not exist in the table, index = %d\n", key, indx);
}

void extendChain(CHAIN **head, int key)
{
    CHAIN *node = NULL;
    node = malloc(sizeof(CHAIN));
    if (node == NULL)
    {
        printf("failed to allocate memory\n");
        abort();
    }
    node->data = key;
    node->next = *head;
    *head = node;
}

void printer(CHAIN *hash[BUCKET_SIZE])
{
    CHAIN *head;
    printf("Bucket\n");
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        printf("[%d] ", i);
        if (hash[i] == NULL)
        {
            printf("\n");
            continue;
        }
        head = hash[i];
        while (head != NULL)
        {
            printf("%d ", head->data);
            head = head->next;
        }
        printf("\n");
    }
}

void create_hash_table(int *p)
{
    int temp;
    for (int i = 0; i < length; i++)
    {
        temp = *(p + i) % BUCKET_SIZE;
        extendChain(&hash[temp], *(p + i));
        chain_size[temp]++;
    }
    free(p);
}

void freeLink(CHAIN *hash[BUCKET_SIZE])
{
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        free(hash[i]);
    }
}

void *mythread(void *arg)
{
    myarg_t *args = (myarg_t *)arg;
    CHAIN *head = args->head;
    for (int k = 0; k < args->start; k++)
    {
        head = head->next;
    }
    for (int j = args->start; j < args->end; j++)
    {
        if (head->data == key)
        {
            //printer(args->hash);
            printf("Thread[%d]: The index of %d is [%d][%d]\n", args->thread_id, key, indx, j);
            return NULL;
        }
        head = head->next;
    }
    printf("Thread[%d]: %d does not exist in the table, index = %d\n", args->thread_id, key, indx);
    return NULL;
}

int main()
{
    create_hash_table(unique_random_integer());
    printer(hash);
    char command = '\0';
    while (1)
    {
        printf("enter command. (s = search, p = print, quit = q) ---");
        scanf(" %c", &command);
        if (command == 's')
        {
            printf("search for?\n");
            scanf(" %d", &key);
            indx = key % BUCKET_SIZE;
            if (indx < 0)
            {
                printf("error\n");
                continue;
            }
            else
            {
                pthread_t pthread;
                myarg_t args[thread_number];
                for (int i = 0; i < thread_number - 1; i++)
                {
                    args[i].head = hash[indx];
                    args[i].start = chain_size[indx] / thread_number * i;
                    args[i].end = chain_size[indx] / thread_number * (i + 1);
                    args[i].thread_id = i + 1;
                }
                for (int i = 0; i < thread_number - 1; i++)
                {
                    pthread_create(&pthread, NULL, mythread, &args[i]);
                }
                hashSearch(hash[indx], key);
                pthread_join(pthread, NULL);
            }
        }
        else if (command == 'p')
        {
            printer(hash);
        }
        else
        {
            break;
        }
    }
    freeLink(hash);
}