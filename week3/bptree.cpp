#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include "../integers.hh"

struct timeval cur_time(void);
void print_tree_core(NODE *n);
void print_tree(NODE *node);
NODE *find_leaf(NODE *node, int key);
template <class LEAF>
LEAF *insert_in_leaf(LEAF *leaf, int key, DATA *data);
NODE *alloc_leaf(NODE *parent);
void insert_in_parent(NODE *leaf, int k, NODE *new_leaf);
template <class PARENT>
void insertToParent(int key, PARENT *parent, NODE *child, NODE *brother);
void insert(int key, DATA *data);
TEMP *copyNode(NODE *node);
void eraseNode(NODE *node);
void init_root(void);
int interactive();
int search(int key, NODE *n);
void rangeSearch(int head, int tail, NODE *n);

struct timeval
cur_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

void print_tree_core(NODE *n)
{
    printf("[");
    for (int i = 0; i < n->nkey; i++)
    {
        if (!n->isLeaf)
            print_tree_core(n->chi[i]);
        printf("%d", n->key[i]);
        if (i != n->nkey - 1 && n->isLeaf)
            putchar(' ');
    }
    if (!n->isLeaf)
        print_tree_core(n->chi[n->nkey]);
    printf("]");
}

void print_tree(NODE *node)
{
    print_tree_core(node);
    printf("\n");
    fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
    int kid;

    if (node->isLeaf)
        return node;
    for (kid = 0; kid < node->nkey; kid++)
    {
        if (key < node->key[kid])
            break;
    }

    return find_leaf(node->chi[kid], key);
}

template <class LEAF>
LEAF *
insert_in_leaf(LEAF *leaf, int key, DATA *data)
{
    int i;
    if (key < leaf->key[0])
    {
        for (i = leaf->nkey; i > 0; i--)
        {
            leaf->chi[i] = leaf->chi[i - 1];
            leaf->key[i] = leaf->key[i - 1];
        }
        leaf->key[0] = key;
        leaf->chi[0] = (NODE *)data;
    }
    else
    {
        for (i = 0; i < leaf->nkey; i++)
        {
            if (key < leaf->key[i])
                break;
        }
        for (int j = leaf->nkey; j > i; j--)
        {
            leaf->chi[j] = leaf->chi[j - 1];
            leaf->key[j] = leaf->key[j - 1];
        }
        leaf->key[i] = key;
        leaf->chi[i] = (NODE *)data;
    }
    leaf->nkey++;

    return leaf;
}

NODE *
alloc_leaf(NODE *parent)
{
    NODE *node;
    if (!(node = (NODE *)calloc(1, sizeof(NODE))))
        ERR;
    node->isLeaf = true;
    node->parent = parent;
    node->nkey = 0;

    return node;
}

TEMP *copyNode(NODE *node)
{
    TEMP *copy;
    if (!(copy = (TEMP *)calloc(1, sizeof(TEMP))))
        ERR;
    for (int i = 0; i < N; i++)
    {
        copy->chi[i] = node->chi[i];
        if (i < N - 1)
            copy->key[i] = node->key[i];
    }
    copy->isLeaf = node->isLeaf;
    copy->nkey = node->nkey;
    return copy;
}

void eraseNode(NODE *node)
{
    for (int i = 0; i < N; i++)
    {
        node->chi[i] = NULL;
        if (i < N - 1)
            node->key[i] = 0;
    }
    node->nkey = 0;
}

void insert_in_parent(NODE *leaf, int k, NODE *new_leaf)
{
    // 1: if N is the root of the tree then
    if (leaf == Root)
    {
        // cout << " insert_in_parent 1" << endl;
        // 2: Create a new node R containing N,K',N'
        NODE *parent;
        if (!(parent = (NODE *)calloc(1, sizeof(NODE))))
            ERR;
        parent->chi[0] = leaf;
        parent->chi[1] = new_leaf;
        parent->key[0] = k;
        parent->nkey++;

        parent->isLeaf = false;
        parent->parent = NULL;
        leaf->parent = parent;
        new_leaf->parent = parent;
        // 3: Make R the root of the tree return
        Root = parent;
        return;
        // 4: end if
    }
    // 5: Let P = parent(N)
    NODE *P = leaf->parent;
    // 6: if P has less than n pointers then
    if (P->nkey + 1 < N)
    {
        // 7: insert(K', N') in P just after N
        insertToParent(k, P, new_leaf, leaf);
        new_leaf->parent = P;
    }
    // 8: else
    else
    {
        // 9: Copy P to a block of memory T that hold P and (K'&N')
        TEMP *copy_parent = copyNode(P);
        // 10: Insert(K', N') into T just after N
        insertToParent(k, copy_parent, new_leaf, leaf);
        // 11: Erase all entries from P
        eraseNode(P);
        // 12: Create node P'
        NODE *new_parent;
        if (!(new_parent = (NODE *)calloc(1, sizeof(NODE))))
            ERR;
        new_parent->isLeaf = false;
        // 13: CopyT.P1,...,T.Pà(n+1)/2à intoP
        // 15: Copy T.Pà(n+1)/2à+1,...,T.Pn+1 into P'
        int j = 0;
        int k = 0;
        for (int i = 0; i < N + 1; i++)
        {
            if (i < ceil((float)(N + 1) / 2))
            {
                P->chi[i] = copy_parent->chi[i];
                P->chi[i]->parent = P;
            }
            else
            {
                new_parent->chi[j] = copy_parent->chi[i];
                new_parent->chi[j]->parent = new_parent;
                j++;
            }
        }
        j = 0;
        for (int i = 0; i < N; i++)
        {
            if (i < ceil((float)(N + 1) / 2) - 1)
            {
                P->key[i] = copy_parent->key[i];
                P->nkey++;
            }
            else if (i == ceil((float)(N + 1) / 2) - 1)
            {
                k = copy_parent->key[i];
            }
            else
            {
                new_parent->key[j] = copy_parent->key[i];
                new_parent->nkey++;
                j++;
            }
        }

        // new leafの後のleafのparentも変更する必要がある
        // 16: insert in parent(P,K',P')
        insert_in_parent(P, k, new_parent);
        // 17: end if
    }
}

template <class PARENT>
void insertToParent(int key, PARENT *parent, NODE *child, NODE *brother)
{
    int i;
    for (i = 0; i <= parent->nkey; i++)
    {
        if (parent->chi[i] == brother)
        {
            for (int j = parent->nkey; j > i; j--)
            {
                parent->chi[j + 1] = parent->chi[j];
                parent->key[j] = parent->key[j - 1];
            }
            parent->key[i] = key;
            parent->chi[i + 1] = child;
        }
    }
    parent->nkey++;
}

void insert(int key, DATA *data)
{
    NODE *leaf;

    if (Root == NULL)
    {
        leaf = alloc_leaf(NULL);
        Root = leaf;
    }
    else
    {
        leaf = find_leaf(Root, key);
    }
    if (leaf->nkey < (N - 1))
    {
        insert_in_leaf(leaf, key, data);
    }
    else
    {
        NODE *new_leaf = alloc_leaf(NULL);
        TEMP *copy_leaf = copyNode(leaf);
        insert_in_leaf(copy_leaf, key, data);
        new_leaf->chi[N - 1] = leaf->chi[N - 1];
        eraseNode(leaf);
        leaf->chi[N - 1] = new_leaf;

        int j = 0;
        for (int i = 0; i < N; i++)
        {
            if (i < N / 2)
            {
                leaf->chi[i] = copy_leaf->chi[i];
                leaf->key[i] = copy_leaf->key[i];
                leaf->nkey++;
            }
            else
            {
                new_leaf->chi[j] = copy_leaf->chi[i];
                new_leaf->key[j] = copy_leaf->key[i];
                j++;
                new_leaf->nkey++;
            }
        }
        int k = new_leaf->key[0];
        insert_in_parent(leaf, k, new_leaf);
    }
}

void init_root(void)
{
    Root = NULL;
}

int interactive()
{
    int key;

    std::cout << "Key: ";
    std::cin >> key;

    return key;
}

int search(int key, NODE *n)
{
    if (n->isLeaf == true)
    {
        for (int i = 0; i < n->nkey; i++)
        {
            if (n->key[i] == key)
                return 1;
        }
        return 0;
    }
    for (int i = 0; i < n->nkey; i++)
    {
        if (key < n->key[i])
            return search(key, n->chi[i]);
        if (i == n->nkey - 1)
            return search(key, n->chi[++i]);
    }
    return 0;
}

void rangeSearch(int head, int tail, NODE *n)
{
    if (n->isLeaf == true)
    {
        for (int i = 0; i < n->nkey; i++)
        {
            if (n->key[i] >= head && n->key[i] <= tail)
            {
                cout << n->key[i] << endl;
            }
        }
        NODE *node = n->chi[N - 1];
        int flag = 0;
        while (flag == 0)
        {
            for (int i = 0; i < node->nkey; i++)
            {
                if (node->key[i] <= tail)
                    cout << node->key[i] << endl;
                else
                {
                    flag = 1;
                    break;
                }
            }
            if (node->chi[N - 1] != NULL)
                node = node->chi[N - 1];
            else
                break;
        }
        return;
    }
    for (int i = 0; i < n->nkey; i++)
    {
        if (head < n->key[i])
            return rangeSearch(head, tail, n->chi[i]);
        if (i == n->nkey - 1)
            return rangeSearch(head, tail, n->chi[++i]);
    }
    return;
}

int main(int argc, char *argv[])
{
    struct timeval begin, end;

    init_root();
    // int *random_integers = unique_random_integer();
    printf("-----Insert-----\n");
    begin = cur_time();
    /*
        while (true)
        {
            insert(interactive(), NULL);
            cout << "printing" << endl;
            print_tree(Root);
        }

        for (int i = 0; i < LENGTH; i++)
        {
            insert(*(random_integers + i), NULL);
        }
        // print_tree(Root);
        end = cur_time();
        cout << "time: " << (end.tv_sec - begin.tv_sec) << endl;

        int sum = 0;
        for (int i = 0; i < LENGTH; i++)
        {
            sum += search(*(random_integers + i), Root);
        }
        cout << "search hit: " << sum << endl;
        free(random_integers);
    */
    char operation = '\0';
    while (true)
    {
        cout << "[i] insert, [s] search, [r] rangeSearch, [d] delete, [p] print: ";
        cin >> operation;
        if (operation == 'i')
        {
            printf("-----Insert-----\n");
            insert(interactive(), NULL);
        }
        else if (operation == 's')
        {
            printf("-----Search-----\n");
            int key = interactive();
            if (search(key, Root))
                cout << key << " found" << endl;
            else
                cout << key << " not found" << endl;
        }
        else if (operation == 'r')
        {
            printf("-----Range Search-----\n");
            int head = interactive();
            int tail = interactive();
            // int max 2147483647
            rangeSearch(head, tail, Root);
        }
        else if (operation == 'p')
        {
            printf("-----Print-----\n");
            print_tree(Root);
        }
        else
            break;
    }
    free(Root);
    return 0;
}

// error
// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 100 99 98 97 96 95

// cout << "  " << leaf->key[0] << "leaf->parent->key[0]: " << leaf->parent->key[0] << endl;
// cout << "  " << new_leaf->key[0] << "new_leaf->parent->key[0]: " << new_leaf->parent->key[0] << endl;
// cout << "  " << leaf->key[0] << "leaf->parent->key[0]: " << leaf->parent->key[0] << endl;
// cout << "  " << new_leaf->key[0] << "new_leaf->parent->key[0]: " << new_leaf->parent->key[0] << endl;
// cout << "leaf->key[0]: " << leaf->key[0] << endl;
// cout << "new_leaf->key[0]: " << new_leaf->key[0] << endl;
// cout << "P->key[0]: " << P->key[0] << endl;
// cout << "P->key[1]: " << P->key[1] << endl;

/*void traversingParentFlip(NODE *parent)
{
    if (parent->isLeaf == false)
    {
        for (int i = 0; i < parent->nkey + 1; i++)
        {
            parent->chi[i]->parent = parent;
            traversingParentFlip(parent->chi[i]);
        }
    }
    return;
}*/

// i 1 i 2 i 3 i 4 i 5 i 6 i 7 i 8 i 9 i 10