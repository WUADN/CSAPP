
1.create data structure store cache with necessary item?
2.design LRU algorithm
3.design input format

## 1.create data structure store cache with necessary item?
input: set-nums, Associativity, Number of block size 
we just follow cache data model: line as basic element in cache ds, 
{
  struct cache_element[S*E];
  int S; 
  int E;
}cache;

{
  int valid;
  uint64_t tag;
  int set_index;
  int block_offset;
}
