#include "storage.h"
#include <cmath>
#include <iostream>
#include <climits>


#include <cstdlib>
#include <chrono>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


Storage::Storage(const std::vector<int>& nodes, const std::vector<int>& edges, int blockSizeVertex, int blockSizeEdge)
    : blockSizeVertex(blockSizeVertex), blockSizeEdge(blockSizeEdge) {
  numEdges = edges.size();
  numNodes = nodes.size();
  edgeIndex.resize(numEdges);
  nodeIndex.resize(numNodes);
  numEdgeBlocks = (numEdges / blockSizeEdge) + (numEdges % blockSizeEdge == 0 ? 0 : 1);
  numNodeBlocks = (numNodes / blockSizeVertex) + (numNodes % blockSizeVertex == 0 ? 0 : 1);
  
  //openStorage("myDBfile");
}

int Storage::getBlockID(int srcID, int vrs) {
  if(vrs==-1){ //edge
    return edgeIndex[srcID];
  }
  else{ //vertex
    return nodeIndex[srcID];
  }
  
}

void Storage::loadData(const std::vector<int>& edges, const std::vector<int>& nodes) {
  int blockID = -1;
  for (int i = 0; i < edges.size(); i++) {
    blockID = i / blockSizeEdge;
    edgeIndex[i] = blockID;
    if(pageIsInCache.find(blockID) == pageIsInCache.end()) {
      DiskPage *page = new DiskPage(blockID,-1,false);
      pageIsInCache[blockID] = page;
    }
  }
  blockID++; 

  for (int i = 0; i < nodes.size(); i++) { 
    int tempblockID = i / blockSizeVertex;
    tempblockID += blockID;
    nodeIndex[nodes[i]] = tempblockID;
    if(pageIsInCache.find(tempblockID) == pageIsInCache.end()){
      DiskPage *page = new DiskPage(tempblockID,-1,false);
      pageIsInCache[blockID] = page;
    }
  }
}

void Storage::openStorage(const char* dbFileName){
  //fd = open(dbFileName, O_RDONLY|O_DIRECT);
  //posix_memalign((void **)&buffer, PAGE_SIZE, (PAGE_SIZE * 2));
}

void Storage::fetchFromStorageOffset(int blockID){
  off_t offset = blockID * PAGE_SIZE;
  pread(fd,buffer,PAGE_SIZE,offset);
}
