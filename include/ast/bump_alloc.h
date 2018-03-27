#ifndef INCLUDE_AST_BUMP_ALLOC_H
#define INCLUDE_AST_BUMP_ALLOC_H

struct bump_alloc_area_header;

struct bump_alloc {
    struct bump_alloc_area_header *areas;
    size_t area_size;
};

#define BUMP_ALLOC_INIT(a_size) \
    { \
        .areas = NULL, \
        .area_size = (a_size), \
    }

static inline void bump_alloc_init(struct bump_alloc *alloc, size_t area_size)
{
    *alloc = (struct bump_alloc)BUMP_ALLOC_INIT(area_size);
}

void bump_alloc_free_all(struct bump_alloc *);

void *bump_alloc_get(struct bump_alloc *, size_t obj_size, void (*clear_func) (void *));

#endif
