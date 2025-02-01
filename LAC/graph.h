#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include "gclock.h"
#include "lac-gc.h"
#include "storage.h"
#include "cache.h"

class Graph {
 public:
  int blockSize;
  int blockSizeVertex;
  int blockSizeEdge;
  int cachePercent;
  int numNodes;
  std::vector<int> index;  // O(V) CSR index
  std::vector<int> nodeOrder;
  std::vector<int> nodes;
  std::vector<int> edges;

  std::vector<int> nodeData;
  std::unordered_map<int, int> nodeDegree;

  Storage* data;
  Cache* cache;

  void readEdgesFromFile(const char* fileName);
  void readNodesFromFile(const char* fileName);

 public:
  Graph(int blockSizeVertex, int blockSizeEdge, int cachePercent);
  ~Graph() {}

  int maxSPLevel;

  int get(int src, int dst);

  int bidiSP(int src, int dst);
  int fr_all_2(int src);
  double fr_all(int src, double p);
  double rw(int src, int NS, int NW);

  void createGraphFromFile(const char* fileName);
  void loadDataToStorage(const char* fileName, int policy);
  long printStats();



};

#endif
