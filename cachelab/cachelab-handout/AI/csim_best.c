#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cachelab.h"

typedef struct {
    int valid;
    uint64_t tag;
    uint64_t last_used;
} Line;

typedef struct {
    int s;
    int E;
    int b;
    uint64_t now;
    Line *lines;
} CacheSim;

static void usage(char *prog)
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", prog);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
}

static void sim_init_cache(CacheSim *cache, int s, int E, int b)
{
    int line_count = (1 << s) * E;

    cache->s = s;
    cache->E = E;
    cache->b = b;
    cache->now = 0;
    cache->lines = calloc(line_count, sizeof(Line));
    if (cache->lines == NULL) {
        perror("calloc");
        exit(1);
    }
}

static int get_set(CacheSim *cache, uint64_t addr)
{
    return (addr >> cache->b) & ((1 << cache->s) - 1);
}

static uint64_t get_tag(CacheSim *cache, uint64_t addr)
{
    return addr >> (cache->s + cache->b);
}

static void access_cache(CacheSim *cache, uint64_t addr, int verbose,
                         int *hits, int *misses, int *evictions)
{
    int set = get_set(cache, addr);
    uint64_t tag = get_tag(cache, addr);
    int begin = set * cache->E;
    int end = begin + cache->E;
    int empty = -1;
    int victim = begin;

    /*
     * LRU is represented by a timestamp. The least recently used line is the
     * valid line with the smallest last_used value.
     */
    cache->now++;

    for (int i = begin; i < end; i++) {
        Line *line = &cache->lines[i];

        if (line->valid && line->tag == tag) {
            (*hits)++;
            line->last_used = cache->now;
            if (verbose) {
                printf(" hit");
            }
            return;
        }

        if (!line->valid && empty == -1) {
            empty = i;
        }

        if (line->last_used < cache->lines[victim].last_used) {
            victim = i;
        }
    }

    (*misses)++;
    if (verbose) {
        printf(" miss");
    }

    if (empty != -1) {
        victim = empty;
    } else {
        (*evictions)++;
        if (verbose) {
            printf(" eviction");
        }
    }

    cache->lines[victim].valid = 1;
    cache->lines[victim].tag = tag;
    cache->lines[victim].last_used = cache->now;
}

static void replay_trace(CacheSim *cache, FILE *fp, int verbose,
                         int *hits, int *misses, int *evictions)
{
    char op;
    uint64_t addr;
    int size;

    while (fscanf(fp, " %c %" SCNx64 ",%d", &op, &addr, &size) == 3) {
        if (op == 'I') {
            continue;
        }

        if (verbose) {
            printf("%c %" PRIx64 ",%d", op, addr, size);
        }

        if (op == 'L' || op == 'S') {
            access_cache(cache, addr, verbose, hits, misses, evictions);
        } else if (op == 'M') {
            /* Modify is a load followed by a store on the same address. */
            access_cache(cache, addr, verbose, hits, misses, evictions);
            access_cache(cache, addr, verbose, hits, misses, evictions);
        }

        if (verbose) {
            printf("\n");
        }
    }
}

int main(int argc, char **argv)
{
    int s = 0;
    int E = 0;
    int b = 0;
    int verbose = 0;
    int has_s = 0;
    int has_E = 0;
    int has_b = 0;
    int hits = 0;
    int misses = 0;
    int evictions = 0;
    char *trace_file = NULL;
    int opt;
    FILE *fp;
    CacheSim cache;

    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
        case 's':
            s = atoi(optarg);
            has_s = 1;
            break;
        case 'E':
            E = atoi(optarg);
            has_E = 1;
            break;
        case 'b':
            b = atoi(optarg);
            has_b = 1;
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    /*
     * s=0 and b=0 are valid cache geometries, so separate flags record whether
     * the required options were present.
     */
    if (!has_s || !has_E || !has_b || E <= 0 || trace_file == NULL) {
        usage(argv[0]);
        return 1;
    }

    fp = fopen(trace_file, "r");
    if (fp == NULL) {
        perror(trace_file);
        return 1;
    }

    sim_init_cache(&cache, s, E, b);
    replay_trace(&cache, fp, verbose, &hits, &misses, &evictions);
    printSummary(hits, misses, evictions);

    free(cache.lines);
    fclose(fp);
    return 0;
}
