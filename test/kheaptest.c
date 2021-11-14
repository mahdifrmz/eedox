#include <kheap.h>
#include <stdio.h>
#include <string.h>

int32_t bin_search(uint32_t *ptr, uint32_t len, uint32_t value);
uint8_t hheader_isvalid(hheader_t *head, heap_t *heap);
hheader_t *hheader_next(hheader_t *head);

void bin_search_test()
{
    uint32_t list[10] = {0, 1, 3, 3, 5, 7, 8, 14, 56, 222};
    int32_t i1 = bin_search(list, 10, 8);
    printf("i1(6)=%d\n", i1);
    int32_t i2 = bin_search(list, 10, 6);
    printf("i2(-1)=%d\n", i2);
}

void heap_print(heap_t *heap, void **keylist, size_t keylist_size)
{
    hheader_t *cur = (hheader_t *)heap->start;
    while (hheader_isvalid(cur, heap))
    {
        void *ptr = (void *)((uint32_t)cur + sizeof(hheader_t));
        int idx = bin_search((uint32_t *)keylist, keylist_size, (uint32_t)ptr);
        printf("|%d:%u(%s)", idx, cur->size, cur->is_hole == 1 ? "free" : "alloced");
        cur = hheader_next(cur);
    }
    printf("|\n");
}

void heap_test_cases()
{
    uint32_t heap_size = 1024 * 16;
    uint32_t index_size = 256;
    char buffer[heap_size + index_size];
    heap_t heap;
    heap_init(&heap, buffer, heap_size, index_size, 0, 0);
    char *a1 = heap_alloc(&heap, 16, 0);
    char *a2 = heap_alloc(&heap, 40, 0);
    char *a3 = heap_alloc(&heap, 16, 0);
    heap_free(&heap, a1);
    char *a4 = heap_alloc(&heap, 16, 0);
    heap_free(&heap, a4);
    heap_free(&heap, a2);
    char *a5 = heap_alloc(&heap, 16 + sizeof(hheader_t) + 40, 0);
    heap_free(&heap, a5);
    heap_free(&heap, a3);

    char *l1 = heap_alloc(&heap, 50, 1);
    char *l2 = heap_alloc(&heap, 100, 1);
    heap_free(&heap, l1);
    heap_free(&heap, l2);
    // char *l3 = heap_alloc(&heap, 50, 1);
}

void heap_test_shell()
{
    uint32_t heap_size = 1024 * 16;
    uint32_t index_size = 256;
    char buffer[heap_size + index_size];
    heap_t heap;
    heap_init(&heap, buffer, heap_size, index_size, 0, 0);

    char command[10];
    uint32_t val;

    void *keylist[1024];
    size_t keylist_size = 0;

    while (1)
    {
        heap_print(&heap, keylist, keylist_size);
        scanf("%s", command);
        scanf("%u", &val);

        if (strcmp(command, "a0") == 0)
        {
            keylist[keylist_size++] = heap_alloc(&heap, val, 0);
        }
        else if (strcmp(command, "a1") == 0)
        {
            keylist[keylist_size++] = heap_alloc(&heap, val, 1);
        }
        else if (strcmp(command, "f") == 0)
        {
            heap_free(&heap, keylist[val]);
        }
        else
        {
            printf("Unknown command\n");
        }
    }
}

int main()
{
    // heap_test_cases();
    heap_test_shell();
}