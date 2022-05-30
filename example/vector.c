#include "vector.h"
#include <string.h>

void vector_init(vector* vec, size_t elem_size) {
    vec->size = 0;
    vec->count = 0;
    vec->elem_size = elem_size;
    vec->elems = NULL;
}

void vector_free(vector* vec) {
    free(vec->elems);
}

void vector_alloc(vector* vec, size_t count) {
    if (vec->size >= count)
        return;

    if (vec->size == 0)
        vec->size = 16;
    while (vec->size < count)
        vec->size *= 2;

    vec->elems = realloc(vec->elems, vec->elem_size * vec->size);
}

void vector_set(vector* vec, size_t idx, void* elem) {
    if (idx + 1 > vec->count) {
        vec->count = idx + 1;
        vector_alloc(vec, vec->count);
    }

    void* ptr = (void*) ((char*) vec->elems + (vec->elem_size * idx));
    memcpy(ptr, elem, vec->elem_size);
}

void* vector_get(vector* vec, size_t idx) {
    if (vec->count <= idx)
        return NULL;

    void* ptr = (void*) ((char*) vec->elems + (vec->elem_size * idx));
    return ptr;
}

#define UNIT_MAIN
#include <unit.h>

suite(vector) {
    vector vec;

    it("breaks the rules of math (to demonstrate failed tests)", .failing = true) {
        require_eq(1, 2, binary op fail);
        echo("Oh, noes!");
        // these checks will be skipped after previous fail
        require_eq(vector_get(&vec, 101), NULL);
        require_eq(vec.elem_size, 53);
    }

    it("inits vectors correctly") {
        vector_init(&vec, 53);

        require_eq(vec.size, 0);
        require_eq(vec.count, 0);
        require_eq(vec.elem_size, 53);
        require_eq(vec.size, 0);

        vector_free(&vec);
    }

#define it_vec(name) it(name) UNIT_SCOPE( \
/* before: */ vector_init(&vec, sizeof(int)), \
/* after:  */ vector_free(&vec) \
)

    it_vec("allocates vectors based on elem_size") {
            vector_alloc(&vec, 10);
            require_eq(vec.elem_size, sizeof(int));
            require_eq(vec.count, 0);
            require(vec.size >= 10);
            /* Not an assert, but will cause valgrind to complain
             * if not enough memory was allocated */
            memset(vec.elems, 0xff, 10 * vec.elem_size);
        }

    describe(vector_set) {
        it_vec("sets values inside of the allocated range") {
                vector_alloc(&vec, 2);
                int el = 10;
                vector_set(&vec, 1, &el);
                require_eq(*(int*) ((char*) vec.elems + sizeof(int)), 10);
            }

        it_vec("allocates space when setting values outside the allocated range") {
                int el = 10;
                vector_set(&vec, 50, &el);
                require_eq(*(int*) ((char*) vec.elems + (sizeof(int) * 50)), 10);
            }
    }

    describe(vector_get) {
        it_vec("gets values inside the allocated range") {
                int el = 500;
                vector_set(&vec, 10, &el);
                require_eq(*(int*) vector_get(&vec, 10), 500);
            }

        it_vec("returns NULL when trying to access values outside the allocated range") {
                int el = 10;
                vector_set(&vec, 100, &el);
                require_eq(vector_get(&vec, 101), NULL);
            }
    }
}
