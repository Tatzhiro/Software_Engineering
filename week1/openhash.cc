#include <iostream>
#include <cassert>
#include <sys/time.h>
#include "integers.hh"

// max is around 2,091,000
#define BUCKET_SIZE (LENGTH)
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

double hashInsert(int bucket[], int *numbers)
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

    double insertion_time = hashInsert(bucket, numbers);
    //printer(bucket);
    std::cout << "insertion time: " << insertion_time << "μs" << std::endl;
    while (true)
    {
        std::cout << "----Search----" << std::endl;
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
        std::cout << "print? (yes = y)";
        std::cin >> c;
        if (c == 'y')
            printer(bucket);
    }
}

/*
---REPORT---
This code is an implementation of open addressing hash.
First, unique random integers of size LENGTH is produced (using integers.hh).
Once that is done, the random integers are hashed and stored in int bucket[BUCKET_SIZE]
which is a simple integer array structure that is filled with -1 initially.
Random integers are hashed by modulo operation by BUCKET_SIZE.
When inserting a number into the bucket, there is a chance that collision occurs.
In such case, DELTA is added to the index of the number so that the number can be
attempted to be inserted to the bucket again but to a different position.
Hash Search is done in a similar way in a sense that the search index is calculated 
first by modulo operation, and when the search key does not match with the inserted value,
DELTA is added so to search for the key until the index repeats itself.


---EXECUTION RESULT---
LENGTH 1M
BUCKET_SIZE 2M
search key: 2013999462
***Result***
bsearch: 109μs
hash: 0μs
hash search + insertion: 0 + 13059 = 13059μs

LENGTH 1M
BUCKET_SIZE 1M
search key: 23702
***Result***
bsearch: 4μs
hash: 1μs
hash search + insertion: 1 + 689797 = 689798μs

LENGTH 1M
BUCKET_SIZE 1M
search key: 1731251102
***Result***
bsearch: 1μs
hash: 0μs
hash search + insertion: 0 + 1 = 1μs

*Comparison between Binary Search and Open Addressing Hash

In most cases, open addressing hash outperforms binary search 
as the complexity of the search is O(1) and O(logN) respectively.
Hash tables' especially more efficient when there are no collisions
between the inserted numbers.

However, when the bucket size is the same as the data size,
the chance of data collisions would be much higher, leading to
more loops to find the open index in the bucket.
Depending on the number of collisions the search key will have,
binary search can outperform hash tables, although hash table is
still more likely to outperform binary search.

Moreover, when the data size is extremely large or the bucket size is small, 
building the hash table will cost great amount of time compared to
the search time of binary search.

In conclusion, hash table is more efficient as the complexity of the search
is ideally O(1) while the complexity of binary search is O(logN). 
Especially, if you want to search through big amount of data several times,
building a hash table would yield less cost in the long term.
In the short term, however, binary search would yield less cost because
the cost of building a hash table can be much higher than few searches
done with binary search algorithms.
*/
