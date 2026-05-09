/*
 * csim.c - Cache simulator for CS:APP Cache Lab
 * Name: <Your Name>
 * LoginID: <Your Login>
 *
 * Simulates a cache with LRU replacement policy using valgrind memory traces.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "cachelab.h"

/* ---------- Cache Data Structures ---------- */
typedef struct {
    int valid;
    unsigned long tag;
    unsigned long lru;   /* access timestamp for LRU */
} cache_line;

typedef struct {
    cache_line *lines;
} cache_set;

static int S, E, B;           /* S = 2^s sets, E = associativity, B = 2^b block size */
static int s, b;
static cache_set *cache;
static unsigned long timer;   /* global logical clock for LRU */

static int hit_count, miss_count, eviction_count;
static int verbose;

/* ---------- Cache Simulation Core ---------- */

/* Initialize the cache with given parameters */
void init_cache() {
    cache = malloc(S * sizeof(cache_set));
    for (int i = 0; i < S; i++) {
        cache[i].lines = calloc(E, sizeof(cache_line));
    }
    timer = 0;
    hit_count = miss_count = eviction_count = 0;
}

/* Free all allocated cache memory */
void free_cache() {
    for (int i = 0; i < S; i++) {
        free(cache[i].lines);
    }
    free(cache);
}

/*
 * Simulate a single data access (load or store).
 * Returns 1 if hit, 0 if miss, and updates hit/miss/eviction counters.
 */
int access_cache(unsigned long address) {
    /* Extract tag, set index, block offset */
    unsigned long tag = address >> (s + b);
    int set_index = (address >> b) & ((1 << s) - 1);

    cache_set *set = &cache[set_index];
    cache_line *lines = set->lines;

    /* Check for a hit */
    for (int i = 0; i < E; i++) {
        if (lines[i].valid && lines[i].tag == tag) {
            lines[i].lru = ++timer;    /* update LRU timestamp */
            hit_count++;
            if (verbose) printf(" hit");
            return 1;
        }
    }

    /* Miss: find a victim line */
    miss_count++;
    if (verbose) printf(" miss");

    int victim = -1;
    /* First, look for an empty line */
    for (int i = 0; i < E; i++) {
        if (!lines[i].valid) {
            victim = i;
            break;
        }
    }

    /* If no empty line, evict the LRU line */
    if (victim == -1) {
        victim = 0;
        for (int i = 1; i < E; i++) {
            if (lines[i].lru < lines[victim].lru)
                victim = i;
        }
        eviction_count++;
        if (verbose) printf(" eviction");
    }

    /* Load the new block into the victim line */
    lines[victim].valid = 1;
    lines[victim].tag = tag;
    lines[victim].lru = ++timer;
    return 0;
}

/*
 * Process one valgrind trace line
 * Only handles 'L', 'S', 'M' (data accesses). Ignores 'I'.
 */
void process_line(char *line) {
    /* Ignore empty lines */
    if (line[0] == '\0') return;

    char op;
    unsigned long addr;
    int size;

    /* valgrind format: [space]operation address,size */
    /* 'I' has NO leading space; data ops have one leading space */
    if (line[0] != ' ') return;   /* skip 'I' instructions */

    sscanf(line, " %c %lx,%d", &op, &addr, &size);

    if (verbose) printf("%c %lx,%d", op, addr, size);

    switch (op) {
        case 'L':
            access_cache(addr);
            break;
        case 'S':
            access_cache(addr);
            break;
        case 'M':
            /* Modify = Load followed by Store */
            access_cache(addr);
            access_cache(addr);
            break;
        default:
            break;
    }
    if (verbose) printf("\n");
}

/* ---------- Main: Argument Parsing and Trace Processing ---------- */
int main(int argc, char **argv) {
    char *trace_file = NULL;
    int opt;

    /* Parse command-line options */
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                S = 1 << s;
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                B = 1 << b;
                break;
            case 't':
                trace_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                return 1;
        }
    }

    if (!trace_file || s < 0 || E <= 0 || b < 0) {
        fprintf(stderr, "Missing or invalid required arguments.\n");
        return 1;
    }

    init_cache();

    FILE *fp = fopen(trace_file, "r");
    if (!fp) {
        perror("fopen");
        free_cache();
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline if present */
        line[strcspn(line, "\n")] = '\0';
        process_line(line);
    }
    fclose(fp);

    printSummary(hit_count, miss_count, eviction_count);
    free_cache();
    return 0;
}