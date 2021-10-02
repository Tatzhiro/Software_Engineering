#include <iostream>

typedef struct list
{
    struct list *next;
    int data;
} LIST;

LIST *link(LIST *head, int i)
{
    LIST *q = (LIST *)malloc(sizeof(LIST));
    q->data = i;
    head->next = q;
    q->next = NULL;
    return q;
}

int main()
{
    LIST *p;
    p = (LIST *)malloc(sizeof(LIST));
    p->data = 1;
    LIST *head = p;
    for (int i = 2; i <= 100; i++)
    {
        head = link(head, i);
    }
    for (LIST *head = p; head != NULL; head = head->next)
    {
        std::cout << head->data << std::endl;
    }
}