#include "cache.h"
#include <iostream>

Cache::Cache(Storage* storage, int capacity, int blockSizeVertex, int blockSizeEdge, double pageOnDisk)
    : storage(storage),
      capacity(capacity),
      pageOnDisk(pageOnDisk),
      curSize(0),
      //blockSize(blockSize),
      blockSizeVertex(blockSizeVertex),
      blockSizeEdge(blockSizeEdge),
      numHit(0),
      numMiss(0) {

}

  long Cache::printStats(){
      printf("Total Miss: %ld\n",numMiss);
      printf("Total Hit: %ld\n",numHit);
      return numMiss;
  }
