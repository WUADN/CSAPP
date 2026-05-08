#include <assert.h>
#include <stdio.h>

#define main csim_main
#include "csim.c"
#undef main

static void test_init_cache_basic(void) {
    Cache cache;
    init_cache(&cache, 2, 4, 3);

    assert(cache.s == 2);
    assert(cache.E == 4);
    assert(cache.b == 3);
    assert(cache.size == 16);
    assert(cache.lines != NULL);

    for (uint32_t i = 0; i < cache.size; i++) {
        assert(cache.lines[i].valid == 0);
        assert(cache.lines[i].tag == 0);
        assert(cache.lines[i].lru_counter == 0);
    }

    free(cache.lines);
}

static void test_init_cache_single_set(void) {
    Cache cache;
    init_cache(&cache, 0, 1, 0);

    assert(cache.s == 0);
    assert(cache.E == 1);
    assert(cache.b == 0);
    assert(cache.size == 1);
    assert(cache.lines != NULL);

    assert(cache.lines[0].valid == 0);
    assert(cache.lines[0].tag == 0);
    assert(cache.lines[0].lru_counter == 0);

    free(cache.lines);
}

int main(void) {
    test_init_cache_basic();
    test_init_cache_single_set();
    puts("init_cache tests passed");
    return 0;
}
