/*
 * csim.c - Cache Lab Part A: configurable cache simulator with LRU.
 *
 * Author: yueqian
 *
 * Replays a valgrind memory trace through a cache parameterized by
 * (s, E, b) and reports total hits, misses, and evictions. Instruction
 * fetches ('I') are ignored; 'M' is treated as a load followed by a store.
 */
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cachelab.h"

typedef struct {
    int      valid;
    uint64_t tag;
    uint64_t lru;       /* logical timestamp; 0 means never used */
} line_t;

static int      verbose;
static int      hits, misses, evictions;
static uint64_t clk_now;

/*
 * Single pass over the set:
 *   - return on hit (refreshing the line's timestamp)
 *   - otherwise track the line with the smallest timestamp as the victim
 * Invalid lines start at lru=0, so they automatically beat any used line
 * in the LRU race — empty-slot fill and LRU eviction collapse into one path.
 */
static void access_set(line_t *set, int E, uint64_t tag) {
    uint64_t now = ++clk_now;
    int victim = 0;
    for (int i = 0; i < E; i++) {
        if (set[i].valid && set[i].tag == tag) {
            set[i].lru = now;
            hits++;
            if (verbose) printf(" hit");
            return;
        }
        if (set[i].lru < set[victim].lru) victim = i;
    }

    misses++;
    if (verbose) printf(" miss");
    if (set[victim].valid) {
        evictions++;
        if (verbose) printf(" eviction");
    }
    set[victim] = (line_t){.valid = 1, .tag = tag, .lru = now};
}

static void usage(const char *prog, int code) {
    FILE *out = code ? stderr : stdout;
    fprintf(out, "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", prog);
    exit(code);
}

int main(int argc, char **argv) {
    int s = -1, E = 0, b = -1, opt;
    char *trace = NULL;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h': usage(argv[0], 0);
            case 'v': verbose = 1; break;
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': trace = optarg; break;
            default:  usage(argv[0], 1);
        }
    }
    if (s < 0 || E <= 0 || b < 0 || !trace) usage(argv[0], 1);

    size_t S = (size_t)1 << s;
    line_t *cache = calloc(S * (size_t)E, sizeof(line_t));
    if (!cache) { perror("calloc"); return 1; }

    FILE *fp = fopen(trace, "r");
    if (!fp) { perror(trace); free(cache); return 1; }

    char op;
    uint64_t addr;
    int size;
    while (fscanf(fp, " %c %" SCNx64 ",%d", &op, &addr, &size) == 3) {
        if (op == 'I') continue;
        if (verbose) printf("%c %" PRIx64 ",%d", op, addr, size);

        uint64_t tag = addr >> (s + b);
        line_t  *set = &cache[((addr >> b) & (S - 1)) * (size_t)E];
        access_set(set, E, tag);
        if (op == 'M') access_set(set, E, tag);

        if (verbose) putchar('\n');
    }

    fclose(fp);
    free(cache);
    printSummary(hits, misses, evictions);
    return 0;
}
