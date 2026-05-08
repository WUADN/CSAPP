#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include "cachelab.h"



int main(int argc, char **argv)
{
  int s = 0, E = 0, b = 0, v = 0;
  int has_s = 0, has_E = 0, has_b = 0;
  char *trace_file = NULL;
  int opt = 0;
  while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
    switch (opt) {
      case 's': s = atoi(optarg); has_s = 1; break;
      case 'E': E = atoi(optarg); has_E = 1; break;
      case 'b': b = atoi(optarg); has_b = 1; break;
      case 't': trace_file = optarg; break;
      case 'v': v = 1; break;
      case 'h': print_help(); exit(0);
      default: print_usage("./csim-ref: Missing required command line argument\n"); exit(1);
    }
  }

  if (!has_s || !has_E || !has_b || E == 0) {
    print_usage("./csim-ref: Missing required command line argument\n"); exit(1);
  }

  if (trace_file == NULL) {
    print_usage("./csim-ref: option requires an argument -- 't'");
    exit(1);
  }

  FILE *fp = fopen(trace_file, "r");
  if (fp == NULL) {
    perror("fopen");
    exit(1);
  }

  Cache *cache = (Cache*)malloc(sizeof(Cache));
  if (cache == NULL) { perror("cache malloc"); exit(1); } 
  init_cache(cache, s, E, b);

  char op;
  uint64_t addr;
  uint32_t size;
  int hit = 0, miss = 0, eviction = 0;
  while (fscanf(fp, " %c %" SCNx64 ",%" SCNx32, &op, &addr, &size) == 3) {
    if (op == 'I') continue;
    
    if (v) { printf("%c %" PRIx64 ",%d", op, addr, size); }

    if (op == 'L') { load_or_store(cache, addr, 0, v, &hit, &miss, &eviction); }
  if (op == 'S') { load_or_store(cache, addr, 1, v, &hit, &miss, &eviction); }
    if (op == 'M') { modify(cache, addr, v, &hit, &miss, &eviction); }

    if (v) {printf("\n");}
  }
  printSummary(hit, miss, eviction);
  // free memory
  fclose(fp);
  free(cache->lines);
  free(cache);
  return 0;
}


void init_cache(Cache *cache, int s, int E, int b) {

  cache->s = s;
  cache->E = E;
  cache->b = b;
  cache->size = (1U << s) * E;

  cache->lines = (CacheLine*)malloc(sizeof(CacheLine) * cache->size);
  if (cache->lines == NULL) { perror("cachelines malloc"); exit(1); } 

  for (uint32_t i = 0; i < cache->size; i++) {
    cache->lines[i].valid = 0;
    cache->lines[i].tag = 0;
    cache->lines[i].lru_counter = 0;
  }

}

// type = 0 load, else store
void load_or_store(Cache* cache, 
          uint64_t addr,
          int type, 
          int verbose, 
          int* hit, 
          int* miss, 
          int* eviction) {
  int set_index = calculate_set(cache, addr);
  uint64_t tag = calculate_tag(cache, addr);
  int begin = set_index * cache->E;
  int end = begin + cache->E;
  int remain_space = cache->E;
  
  for (int i = begin; i < end; i++) {
  // printf("---size: %" PRIx32 " begin: %d end: %d i: %d------\n", cache->size, begin, end, i);
    if (cache->lines[i].valid) {
      remain_space -= 1;
      if (cache->lines[i].tag == tag) {
        lru_update(cache, addr, 1, i);
        *hit += 1;
        if (verbose) printf(" hit");
        return;
      }
    }
  }

  lru_update(cache, addr, 0, -1);

  *miss += 1;
  if (verbose) printf(" miss");
  if (!remain_space) {
    *eviction += 1;
    if (verbose) printf(" eviction");
  }
  return;
}

void modify(Cache* cache, 
          uint64_t addr,
          int verbose, 
          int* hit, 
          int* miss, 
          int* eviction) {
  load_or_store(cache, addr, 0, verbose, hit, miss, eviction);
  load_or_store(cache, addr, 1, verbose, hit, miss, eviction);
}
// 由于不用写入数据，逻辑可以统一为：找到地址->更新tag、valid、lru_counter
// hit 不用遍历得到新的地址
void lru_update(Cache *cache, uint64_t addr, int hit, int hit_addr){
  int new_addr = -1;

  int set_index = calculate_set(cache, addr);
  int begin = set_index * cache->E;
  int end = begin + cache->E;
  uint64_t tag = calculate_tag(cache, addr);

  if (!hit) {
    int max_line_index = begin;
    int find_free = 0;
    for (int i = begin; i < end; i++) {
      max_line_index = cache->lines[max_line_index].lru_counter < cache->lines[i].lru_counter ? i : max_line_index; 
      if (!cache->lines[i].valid) {
        new_addr = i;
        find_free = 1;
        break; 
      }
    }

    // 假如没有空闲块
    if (!find_free) {
      new_addr = max_line_index;
    }
  }
  // update tag, valid, lru_counter
  // if (addr == 0x7fefe05a0) printf("\naddr: %" PRIx64 "\n", new_addr);

  if (hit) new_addr = hit_addr;
  
  cache->lines[new_addr].valid = 1;
  cache->lines[new_addr].tag = tag;
  cache->lines[new_addr].lru_counter = 0;
  // int size = 1U << cache->E;

  for (int j = begin; j < end; j++) {
    if (j == new_addr) continue;
    cache->lines[j].lru_counter++;
  }
  
}

void print_help() {
  printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n");
  printf("\n");
  printf("Examples:\n");
  printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
  printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void print_usage(const char* message) {
  printf("%s", message);
  print_help();
}

uint64_t calculate_set(Cache *cache, uint64_t addr) {
  return (int)((addr >> cache->b) & ((1U << cache->s) - 1));
}
uint64_t calculate_tag(Cache *cache, uint64_t addr) {
  return addr >> (cache->s + cache->b);
}
