#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#define BUCKET_SIZE 100000
#define length 1000000

#define lower 1
#define upper 10

// (upper - lower) has to be greater than length

typedef struct chain
{
    int data;
    struct chain *next;
} CHAIN;

typedef struct
{
    CHAIN **head;
    int thread_id;
    int start;
    int end;
    int *list;
} myarg_t;

void create_hash_table(int *p);
void printer(CHAIN **hash);

pthread_mutex_t lock[BUCKET_SIZE];
CHAIN *hash[BUCKET_SIZE];
int thread_number = 1;
int contention[10];

int createRandom()
{
    return rand();
    //return (rand() % (upper - lower + 1)) + lower;
}

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
            r = createRandom();
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
                    r = createRandom();
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

void countIndex()
{
    CHAIN *head = NULL;
    int size_of_index;
    int hash_size = 0;
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        head = hash[i];
        size_of_index = 0;
        while (head != NULL)
        {
            head = head->next;
            size_of_index++;
        }
        printf("|[%d]| %d\n", i, size_of_index);
        hash_size += size_of_index;
    }
    printf("size of hash = %d\n", hash_size);
}

void hashSearch(CHAIN *head, int key, int index)
{
    int i = 0;
    while (head != NULL)
    {
        if (head->data == key)
        {
            printf("Main: The index of %d is [%d][%d]\n", key, index, i);
            return;
        }
        head = head->next;
        i++;
    }
    printf("%d does not exist in the table, index = %d\n", key, index);
}

void extendChain(int temp, int key)
{
    CHAIN *node = NULL;
    CHAIN **head = NULL;
    node = malloc(sizeof(CHAIN));
    if (node == NULL)
    {
        printf("failed to allocate memory\n");
        abort();
    }
    if (pthread_mutex_trylock(&lock[temp]) != 0)
    {
        contention[0]++;
        usleep(5);
        extendChain(temp, key);
    }
    else
    {
        head = &hash[temp];
        node->data = key;
        node->next = *head;
        *head = node;
        pthread_mutex_unlock(&lock[temp]);
    }
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

void *mythread(void *arg)
{
    myarg_t *args = (myarg_t *)arg;
    CHAIN **head = NULL;
    CHAIN *node = NULL;
    int temp;
    int *p = args->list;
    for (int i = args->start; i < args->end; i++)
    {
        node = NULL;
        node = malloc(sizeof(CHAIN));
        if (node == NULL)
        {
            printf("failed to allocate memory\n");
            abort();
        }
        temp = *(p + i) % BUCKET_SIZE;
        if (pthread_mutex_trylock(&lock[temp]) != 0)
        {
            contention[args->thread_id]++;
            i--;
            usleep(5);
        }
        else
        {
            head = &hash[temp];
            node->data = *(p + i);
            node->next = *head;
            *head = node;
            pthread_mutex_unlock(&lock[temp]);
        }
    }
    //printf("thread[%d] done, contention = %d\n", args->thread_id, contention[args->thread_id]);
    return NULL;
}

void create_hash_table(int *p)
{
    while (thread_number < 10)
    {
        struct timespec start, finish;
        double elapsed = 0;
        int sumcontention = 0;
        for (int i = 0; i < BUCKET_SIZE; i++)
        {
            hash[i] = NULL;
        }
        int temp;
        pthread_t pthread[thread_number - 1];
        myarg_t args[thread_number - 1];
        for (int i = 0; i < thread_number - 1; i++)
        {
            args[i].start = length / thread_number * i;
            args[i].end = length / thread_number * (i + 1);
            args[i].thread_id = i + 1;
            args[i].list = p;
        }
        for (int i = 0; i < thread_number - 1; i++)
        {
            pthread_create(&pthread[i], NULL, mythread, &args[i]);
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = length / thread_number * (thread_number - 1); i < length; i++)
        {
            temp = *(p + i) % BUCKET_SIZE;
            extendChain(temp, *(p + i));
        }
        //printf("main done, contention = %d\n", contention[0]);
        for (int i = 0; i < thread_number - 1; ++i)
        {
            pthread_join(pthread[i], NULL);
        }
        //printf("main done\n");
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        for (int i = 0; i < thread_number; i++)
        {
            sumcontention += contention[i];
            contention[i] = 0;
        }
        printf("%d,%f, %d\n", thread_number, elapsed, sumcontention);
        //printf("num thread [%d] time: %f; sum of contentions = %d\n", thread_number, elapsed, sumcontention);
        thread_number++;
    }
}

void freeLink(CHAIN *hash[BUCKET_SIZE])
{
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        free(hash[i]);
    }
}

int main()
{
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        pthread_mutex_init(&lock[i], NULL);
    }
    int *random_integers = unique_random_integer();
    create_hash_table(random_integers);
    free(random_integers);
    //printer(hash);
    char command = '\0';
    int key = 0;
    int index = 0;
    while (1)
    {
        printf("enter command. (s = search, p = print, c = count, quit = q) ---");
        scanf(" %c", &command);
        if (command == 's')
        {
            printf("search for?\n");
            scanf(" %d", &key);
            index = key % BUCKET_SIZE;
            if (index < 0)
            {
                printf("error\n");
                continue;
            }
            else
            {
                hashSearch(hash[index], key, index);
            }
        }
        else if (command == 'c')
        {
            countIndex();
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