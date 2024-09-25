#include "cache.h"
#include <iostream>

Cache::Cache(Storage* storage, int capacity, int blockSize, double pageOnDisk)
    : storage(storage),
      capacity(capacity),
      pageOnDisk(pageOnDisk),
      curSize(0),
      blockSize(blockSize),
      numHit(0),
      numMiss(0) {

}

// currently LRU, eviction can be seperated


  long Cache::printStats(){
      printf("Total Miss: %ld\n",numMiss);
      printf("Total Hit: %ld\n",numHit);

      return numMiss;
  }
