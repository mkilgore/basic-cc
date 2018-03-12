#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H

#include <stdlib.h>

#if (__STDC_VERSION >= 20112L)
# include <stdalign.h>
# define OBJECT_POOL_MAX_ALIGNMENT alignof(max_align_t)
#else
# define OBJECT_POOL_MAX_ALIGNMENT 16
#endif

struct object_block;

struct object_pool {
    size_t object_size;
    size_t pool_size;
    struct object_block *head;
};

static inline size_t object_pool_align_to_power_of_two(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

#define OBJECT_POOL_INIT(obj_size, pol_size) \
    { \
        .object_size = object_pool_align_to_power_of_two((obj_size), OBJECT_POOL_MAX_ALIGNMENT), \
        .pool_size = (pol_size), \
    }

/*
 * pool_size denotes the number of objects in each pool.
 *
 * The memorypool itself can hold infinite pools, so there is no limit on the
 * amount of objects you can allocate.
 */
static inline void object_pool_init(struct object_pool *pool, size_t object_size, size_t pool_size)
{
    *pool = (struct object_pool)OBJECT_POOL_INIT(object_size, pool_size);
}

/*
 * get and put are used to get objects from the object_pool that can then be used.
 * Putting an object will allow it to be reused by the memory pool.
 *
 * Note: It is *not* necessary to 'put' objects allocated using 'get'.
 * All objects allocated by this pool will be freed back to the system when
 * object_pool_free is called. 'put' is use useful to keep the total
 * memory-usage of the pool down if you know objects won't be used again.
 */
void *object_pool_get(struct object_pool *pool);

/* 
 * Releases all of the allocated objects and object_pool itself.
 */
void object_pool_clear(struct object_pool *pool);

#endif
