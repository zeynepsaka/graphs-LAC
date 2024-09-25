#ifndef CACHE_H
#define CACHE_H

#include "storage.h"
#include <unordered_map>


class Cache {
 public:
  Storage *storage;
  int blockSize;
  int blockSizeVertex;
  int blockSizeEdge;
  const int capacity;
  const float pageOnDisk;
  int curSize;

  // statistics
  long numHit;
  long numMiss;
  
  void resetStats() {
    numHit = 0;
    numMiss = 0;
  }

 public:
  Cache(Storage* storage, int capacity, int blockSizeVertex, int blockSizeEdge, double pageOnDisk);
  ~Cache(){}
  virtual int get(int src, int dst) = 0;
  virtual long printStats();
};

#endif  // CACHE_H
