#include "storage.h"
#include <cmath>
#include <iostream>
#include <climits>


#include <cstdlib>
#include <chrono>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


Storage::Storage(const std::vector<int>& nodes, const std::vector<int>& edges, int blockSize,const std::unordered_map<int, int>& nodeDegree)
    : blockSize(blockSize) {
  numEdges = edges.size();
  numNodes = nodes.size();
  edgeIndex.resize(numEdges);
  nodeIndex.resize(numNodes);
  numEdgeBlocks = (numEdges / 1024) + (numEdges % 1024 == 0 ? 0 : 1);
  numNodeBlocks = (numNodes / 512) + (numNodes % 512 == 0 ? 0 : 1);
  
  for (const std::pair<const int, int> &pair_elem: nodeDegree) {
    graphDegree += pair_elem.second;
  }
  graphDegree = graphDegree/nodes.size();
  openStorage("/home/zkorkmaz/LAC-GC/datasets/myfile");
}

int Storage::getBlockID(int srcID, int vrs) {
  if(vrs==-1){ //edge
    return edgeIndex[srcID];
  }
  else{ //vertex
    return nodeIndex[srcID];
  }
  
}

int Storage::getLevel(int blockID){
  return pageIsInCache[blockID]->level;
}

void Storage::loadData(const std::vector<int>& edges, const std::vector<int>& nodes, const std::unordered_map<int, int>& nodeDegree) {
  int blockID = 0;

  for (int i = 0; i < nodes.size(); i++) { //nodes pages will be serialized after edges (continious)
    int tempblockID = i / 512;
    tempblockID += blockID;
    nodeIndex[nodes[i]] = tempblockID;
    //Page's level will be determined according to the first vertex in that page. 
    //Since the graph is order using Gorder + DBG, consecutive vertices are guarenteed to be in the same level of degree.
    if(pageIsInCache.find(tempblockID) == pageIsInCache.end()){ 
      int degree = -1;
      auto it = nodeDegree.find(nodes[i]);
      if (it != nodeDegree.end()) {
        degree = it->second;
      }
      int level = 1;
      //Since K' default value equals 4 in GCLOCK, we set 4 levels for GRASP.
      // if(degree >= 32*graphDegree) level = 8;
      // else if(degree >= 16*graphDegree && degree<32*graphDegree) level = 7;
      // else if(degree >= 8*graphDegree && degree<16*graphDegree) level = 6;
      // else if(degree >= 4*graphDegree && degree<8*graphDegree) level = 5;
      // else if(degree >= 2*graphDegree && degree<4*graphDegree) level = 4;
      // else if(degree >= 1*graphDegree && degree<2*graphDegree) level = 3;
      // else if(degree >= graphDegree/2 && degree<1*graphDegree) level = 2;
      if(degree >= 16*graphDegree) level = 4;
      else if(degree >= 4*graphDegree && degree<16*graphDegree) level = 3;
      else if(degree >= 1*graphDegree && degree<4*graphDegree) level = 2;
      
      DiskPage *page = new DiskPage(tempblockID,level,false);
      pageIsInCache[tempblockID] = page;
    }
    //pageData[tempblockID].push_back(nodes[i]);
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
