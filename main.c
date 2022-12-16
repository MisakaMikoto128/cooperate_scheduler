#include "cooperate_scheduler_test.h"
#include "sc_list.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    cooperate_scheduler_test();
}

typedef struct
{
    int i;
    struct sc_list next;
} IntList;

void sc_list_test()
{
    struct sc_list list;
    sc_list_init(&list);

    struct sc_list *it = NULL;
    IntList *node = NULL;

    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");

    IntList arr[5] = {{.i = 0}, {.i = 1}, {.i = 2}, {.i = 3}, {.i = 4}};
    sc_list_init(&arr[0].next);
    sc_list_add_tail(&list, &arr[0].next);
    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");

    struct sc_list *poped = sc_list_pop_head(&list);
    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");
    sc_list_add_tail(&list, poped);
    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");

    sc_list_init(&arr[1].next);
    sc_list_add_tail(&list, &arr[1].next);

    sc_list_init(&arr[2].next);
    sc_list_add_tail(&list, &arr[2].next);

    sc_list_init(&arr[3].next);
    sc_list_add_tail(&list, &arr[3].next);
    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");
    poped = sc_list_pop_head(&list);
    sc_list_add_tail(&list, poped);
    sc_list_foreach(&list, it)
    {
        node = sc_list_entry(it, IntList, next);
        printf("[%d]->", node->i);
    }
    puts("");
}
