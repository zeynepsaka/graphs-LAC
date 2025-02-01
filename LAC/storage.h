#ifndef STORAGE_H
#define STORAGE_H

#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <unordered_map>

#define PAGE_SIZE 4096

struct DiskPage{
    int pageID;
    int level;
    bool inCache;
    DiskPage(int p_pageID, int p_level, bool p_inCache):pageID(p_pageID),level(p_level), inCache(p_inCache){}
};

class Storage {
 public:
  int numEdges;
  int numNodes;
  int blockSize;
  int blockSizeEdge;
  int blockSizeVertex;
  int numEdgeBlocks;
  int numNodeBlocks;
  std::vector<int> edgeIndex;               
  std::vector<int> nodeIndex;
  std::map<int,DiskPage*> pageIsInCache;


  unsigned char* buffer;
  int fd;
  
 public:
  Storage(const std::vector<int>& nodes, const std::vector<int>& edges, int blockSizeVertex, int blockSizeEdge);

  ~Storage() {}

  std::vector<int> getEdgeDatabyEdgeID(int src, int dst, int& blockID);
  void openStorage(const char* dbFileName);
  void fetchFromStorageOffset(int offset);
  void loadData(const std::vector<int>& edges, const std::vector<int>& nodes);  
  int getNumEdgeBlocks() { return numEdgeBlocks; }
  int getBlockID(int src, int dst);
  int getNumNodeBlocks() { return numNodeBlocks; }

};

#endif  // STORAGE_H