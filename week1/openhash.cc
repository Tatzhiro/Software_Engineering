#include <iostream>
#include <cassert>
#include <sys/time.h>
#include "integers.hh"

// max is around 2,091,000
#define BUCKET_SIZE (LENGTH * 2)
#define DELTA 3

struct timeval
cur_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

void printer(int hash[])
{
    printf("\nBucket\n");
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        printf("[%d] ", i);
        if (hash[i] >= 0)
        {
            printf("%d", hash[i]);
        }
        printf("\n");
    }
}

float hashInsert(int bucket[], int *numbers)
{
    struct timeval begin, end;
    std::cout << "----insert----" << std::endl;
    begin = cur_time();
    for (int i = 0; i < LENGTH; i++)
    {
        int index = *(numbers + i) % BUCKET_SIZE;
        if (bucket[index] == -1)
            bucket[index] = *(numbers + i);
        else
        {
            while (true)
            {
                index = (index + DELTA) % BUCKET_SIZE;
                if (bucket[index] == -1)
                {
                    bucket[index] = *(numbers + i);
                    break;
                }
                else if (index == *(numbers + i) % BUCKET_SIZE)
                {
                    std::cout << "cannot insert" << std::endl;
                    abort();
                }
            }
        }
    }
    end = cur_time();
    return end.tv_usec - begin.tv_usec;
}

float calculateBSearchTime(int *numbers, int key)
{
    struct timeval bsearch_begin, bsearch_end;
    bsearch_begin = cur_time();
    if (binarysearch(numbers, key, LENGTH) == 1)
    {
        bsearch_end = cur_time();
        return bsearch_end.tv_usec - bsearch_begin.tv_usec;
    }
    else
    {
        std::cout << "bsearch = failure" << std::endl;
        return 0;
    }
}

float hashSearch(int bucket[], int key)
{
    struct timeval begin, end;
    begin = cur_time();
    int indx = key % BUCKET_SIZE;
    while (true)
    {
        if (bucket[indx] == key)
        {
            end = cur_time();
            return (end.tv_usec - begin.tv_usec);
        }
        else
        {
            indx = (indx + DELTA) % BUCKET_SIZE;
            if (indx == key % BUCKET_SIZE)
            {
                std::cout << "hash search = failure" << std::endl;
                return 0;
            }
        }
    }
}

int main()
{
    cout << "unique random integers" << endl;
    int *numbers = unique_random_integer();
    int bucket[BUCKET_SIZE];
    for (int i = 0; i < BUCKET_SIZE; i++)
    {
        bucket[i] = -1;
    }

    float insertion_time = hashInsert(bucket, numbers);
    //printer(bucket);
    std::cout << "insertion time: " << insertion_time << "μs" << std::endl;
    while (true)
    {
        int key;
        std::cout << "key: ";
        std::cin >> key;
        assert(key >= 0);
        float bsearch_time = calculateBSearchTime(numbers, key);
        float hash_time = hashSearch(bucket, key);
        if (hash_time == 0 && bsearch_time == 0)
            printf("search failure\n");
        else
        {
            std::cout << "Search Successful" << std::endl;
            std::cout << "----Result----" << std::endl;
            std::cout << "bsearch: " << bsearch_time << "μs" << std::endl;
            std::cout << "hash: " << hash_time << "μs" << std::endl;
            std::cout << "hash search + insertion: " << hash_time << " + " << insertion_time << " = " << hash_time + insertion_time << "μs" << std::endl;
        }
        char c = '\0';
        std::cout << "print? ";
        std::cin >> c;
        std::cout << std::endl;
        if (c == 'y')
            printer(bucket);
    }
}