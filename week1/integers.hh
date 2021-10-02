#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

#define LENGTH 10

#define lower 1
#define upper 1 * LENGTH

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

//tail is the number of elements of x
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
    int *p = (int *)calloc(LENGTH, sizeof(int));
    int *cp = (int *)calloc(LENGTH, sizeof(int));
    if (p == NULL || cp == NULL)
    {
        printf("failed to allocate memory\n");
        abort();
    }
    else
    {
        srand(time(NULL));
        for (int i = 0; i < LENGTH; i++)
        {
            r = createRandom();
            *(p + i) = r;
            *(cp + i) = r;
        }
        qsort(p, LENGTH, sizeof(int), (int (*)(const void *, const void *))cmp);
        for (int i = 1; i < LENGTH; i++)
        {
            if (*(p + i - 1) == *(p + i))
            {
                while (1)
                {
                    r = createRandom();
                    flag = binarysearch(p, r, LENGTH);
                    if (flag == -1)
                    {
                        search(cp, LENGTH, *(p + i), r);
                        *(p + i) = r;
                        break;
                    }
                }
                insertionSort(p, LENGTH);
                i--;
            }
        }
        // p is sorted unique random integers
        free(cp);
    }
    cout << "unique random integers completed" << endl
         << "Length: " << LENGTH << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << *(p + i) << endl;
    }
    // cp is unsorted unique random integers
    return p;
}