
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ast/bump_alloc.h"

/* Oops... I haven't actually implemented this yet, it's very inefficent */

struct bump_alloc_area_header {
    struct bump_alloc_area_header *next;
    void (*clear_func) (void *);
    void *obj;
};

void bump_alloc_free_all(struct bump_alloc *alloc)
{
    struct bump_alloc_area_header *area = alloc->areas;
    struct bump_alloc_area_header *next_area;

    for (; area; area = next_area) {
        next_area = area->next;

        if (area->clear_func)
            (area->clear_func) (area->obj);

        free(area->obj);
        free(area);
    }

    alloc->areas = NULL;
}

void *bump_alloc_get(struct bump_alloc *alloc, size_t obj_size, void (*clear_func) (void *))
{
    struct bump_alloc_area_header *header = malloc(sizeof(*header));
    memset(header, 0, sizeof(*header));

    header->clear_func = clear_func;
    header->obj = malloc(obj_size);

    header->next = alloc->areas;
    alloc->areas = header;

    return header->obj;
}

