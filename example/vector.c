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
#define UNIT_NO_TIME
//#define UNIT_DEFAULT_ARGS "--ascii", "--trace"
#include <unit.h>

SUITE(vector) {
    vector vec;

    IT("breaks the rules of math (to demonstrate failed tests)", .failing = true) {
        REQUIRE_EQ(1, 2, binary op fail);
        ECHO("Oh, noes!");
        // these checks will be skipped after previous fail
        REQUIRE_EQ(vector_get(&vec, 101), NULL);
        REQUIRE_EQ(vec.elem_size, 53);
    }

    IT("inits vectors correctly") {
        vector_init(&vec, 53);

        REQUIRE_EQ(vec.size, 0);
        REQUIRE_EQ(vec.count, 0);
        REQUIRE_EQ(vec.elem_size, 53);
        REQUIRE_EQ(vec.size, 0);

        vector_free(&vec);
    }

#define IT_VEC(name) IT(name) UNIT_SCOPE( \
/* before: */ vector_init(&vec, sizeof(int)), \
/* after:  */ vector_free(&vec) \
)

    IT_VEC("allocates vectors based on elem_size") {
            vector_alloc(&vec, 10);
            REQUIRE_EQ(vec.elem_size, sizeof(int));
            REQUIRE_EQ(vec.count, 0);
            REQUIRE(vec.size >= 10);
            /* Not an assert, but will cause valgrind to complain
             * if not enough memory was allocated */
            memset(vec.elems, 0xff, 10 * vec.elem_size);
        }

    DESCRIBE(vector_set) {
        IT_VEC("sets values inside of the allocated range") {
                vector_alloc(&vec, 2);
                int el = 10;
                vector_set(&vec, 1, &el);
                REQUIRE_EQ(*(int*) ((char*) vec.elems + sizeof(int)), 10);
            }

        IT_VEC("allocates space when setting values outside the allocated range") {
                int el = 10;
                vector_set(&vec, 50, &el);
                REQUIRE_EQ(*(int*) ((char*) vec.elems + (sizeof(int) * 50)), 10);
            }
    }

    DESCRIBE(vector_get) {
        IT_VEC("gets values inside the allocated range") {
                int el = 500;
                vector_set(&vec, 10, &el);
                REQUIRE_EQ(*(int*) vector_get(&vec, 10), 500);
            }

        IT_VEC("returns NULL when trying to access values outside the allocated range") {
                int el = 10;
                vector_set(&vec, 100, &el);
                REQUIRE_EQ(vector_get(&vec, 101), NULL);
            }
    }
}
