#ifndef CACHE_H
#define CACHE_H

#include "storage.h"
#include <unordered_map>


class Cache {
 public:
  Storage *storage;
  int blockSize;
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
  Cache(Storage* storage, int capacity, int blockSize, double pageOnDisk);
  ~Cache(){}
  virtual int get(int src, int dst) = 0;
  virtual long printStats(); //return the miss count
};

#endif  // CACHE_H
