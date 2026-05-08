/* 
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#include <stdint.h>
#define MAX_TRANS_FUNCS 100

typedef struct trans_func{
  void (*func_ptr)(int M,int N,int[N][M],int[M][N]);
  char* description;
  char correct;
  unsigned int num_hits;
  unsigned int num_misses;
  unsigned int num_evictions;
} trans_func_t;

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */

/* Fill the matrix with data */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
void registerTransFunction(
    void (*trans)(int M,int N,int[N][M],int[M][N]), char* desc);


/*************************csim.c ******************************** */

// it's seems dirty bit not useful in part A
typedef struct {
  int valid;
  uint64_t tag;
  int lru_counter;
} CacheLine;

typedef struct {
  CacheLine *lines;
  uint32_t size;
  uint32_t s; // 2^s set
  uint32_t E; // E line each set
  uint32_t b; // 2^b byte per Block
} Cache;
void print_help(void);
void print_usage(const char *);
void init_cache(Cache *, int, int, int b);
void load_or_store(Cache*, uint64_t, int, int, int*, int*, int*);
void modify(Cache*, uint64_t, int, int*, int*, int*);
void lru_update(Cache *, uint64_t, int, int);
uint64_t calculate_set(Cache*, uint64_t);
uint64_t calculate_tag(Cache*, uint64_t);
#endif /* CACHELAB_TOOLS_H */
