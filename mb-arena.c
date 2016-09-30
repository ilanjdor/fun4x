#include "membench.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

/*

Write an arena allocator for `chunk` objects. Your allocator should:

* Allocate a chunk in O(1) time.
* Free a chunk in O(1) time.
* Use memory proportional to the peak number of actively allocated
chunks (rather than, say, the total number of allocated chunks).
* Run out of memory only if the system has no more memory available.

More on arenas:
https://en.wikipedia.org/wiki/Region-based_memory_management

See "membench.h" for function semantics.

*/

typedef union chunk_or_free {
    chunk c;
    union chunk_or_free* next;
} chunk_or_free;

#define GROUPSIZE 4096
typedef struct membench_group {
    chunk_or_free chunks[GROUPSIZE];
    struct membench_group* next;
} membench_group;

struct membench_arena {
    membench_group* group;
    chunk_or_free* free_list;
};


static membench_group* membench_group_new(membench_arena* a) {
    membench_group* g = (membench_group*) malloc(sizeof(membench_group));
    for (size_t i = 0; i < GROUPSIZE; i++) {
        g->chunks[i].next = a->free_list;
        a->free_list = &g->chunks[i];
    }
    return g;
}

membench_arena* membench_arena_create(void) {
    membench_arena* arena = (membench_arena*) malloc(sizeof(membench_arena));
    arena->free_list = NULL;
    arena->group = membench_group_new(arena);
    return arena;
}

chunk* membench_alloc(membench_arena* arena) {
    if (!arena->free_list) {
        // allocate new group, add it to singly-linked group list
        membench_group* g = membench_group_new(arena);
        g->next = arena->group;
        arena->group = g;
    }

    chunk* result = &arena->free_list->c;
    arena->free_list = arena->free_list->next;
    return result;
}

void membench_free(membench_arena* arena, chunk* x) {
    chunk_or_free* cf = (chunk_or_free*) x;
    cf->next = arena->free_list;
    arena->free_list = cf;
}

void membench_arena_destroy(membench_arena* arena) {
    membench_group* g;
    while ((g = arena->group)) {
        arena->group = g->next;
        free(g);
    }
    free(arena);
}
