#include "graph.h"
#include "storage.h"

#include <algorithm>
#include <fstream>
#include <queue>
#include <stack>
#include <set>
#include <iostream>
#include <chrono>
#include "cache.h"


using namespace std;
Graph::Graph(int blockSize, int cachePercent)
    : blockSize(blockSize), cachePercent(cachePercent) {}

void Graph::readEdgesFromFile(const char* fileName) {
  ifstream infile(fileName);
  int degree = 0;

  int src, dst;
  int prev = -1;
  int cnt = 0;
  int maxId = 0;
  while (infile >> src >> dst) {
    maxId = max(maxId, max(src, dst));
    if (src != prev) {
      if (cnt != 0) {
        index.push_back(cnt);
        nodes.push_back(prev);
      }

      nodeDegree.insert(std::make_pair(prev, degree)); 
      degree=0;

      prev = src;
    }
    edges.push_back(dst);
    cnt++;
    degree++;
  }

  index.push_back(cnt);
  nodes.push_back(prev);

  nodeOrder.resize(maxId + 1, -1);
  for (int i = 0; i < nodes.size(); i++) {
    nodeOrder[nodes[i]] = i;
  }
}

void Graph::readNodesFromFile(const char* fileName) {
  ifstream infile(fileName);

  int node;
  while (infile >> node) {
    nodeData.push_back(node);
  }
}

void Graph::createGraphFromFile(const char* fileName) {
  readEdgesFromFile(fileName);
}

void Graph::loadDataToStorage(const char* fileName, int policy) {
  readNodesFromFile(fileName);
  data = new Storage(nodeData, edges, blockSize, nodeDegree);
  data->loadData(edges, nodeData, nodeDegree);
  
  int cacheCapacity = ((data->getNumNodeBlocks()) * cachePercent) / 100;
  cacheCapacity += ((data->getNumNodeBlocks()) * cachePercent) % 100 == 0 ? 0 : 1;

  //printf("number of page on disk %d, number of page in cache %d\n", data->getNumNodeBlocks(), cacheCapacity);

  
  if(policy == 1){
    cache = new LACGCCache(data, cacheCapacity, blockSize, data->getNumNodeBlocks()); //data->getNumEdgeBlocks()
  }
  else if(policy == 2){
    cache = new GClockCache(data, cacheCapacity, blockSize, data->getNumEdgeBlocks());
  }
  else if(policy == 3){
    cache = new GraspGClockCache(data, cacheCapacity, blockSize, data->getNumNodeBlocks());
  }
  
}

int Graph::get(int src, int dst) {
  int pid = cache->get(src, dst);
  return pid;
}


double Graph::fr_all(int src, double p){
  
  int pid = -1;
  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds; 
  auto t1 = high_resolution_clock::now();
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;
  double time = 0;

  t1 = high_resolution_clock::now();
  pid = get(src, -2);
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  time += ms_double.count();
  
  
  if (nodeOrder[src] == -1) {
    return 0;
  }
  int st = nodeOrder[src] == 0 ? 0 : index[nodeOrder[src] - 1];
  int en = index[nodeOrder[src]];
  
  for (int i = st+1; i < en; i++) {
    int degree = -1;
    std::unordered_map<int, int>::iterator it = nodeDegree.find(edges[i]);
    if (it != nodeDegree.end()) {
      degree = it->second;
    }
    if(degree>p){
      
      t1 = high_resolution_clock::now();
      pid = get(edges[i], -2);
      t2 = high_resolution_clock::now();
      ms_double = t2 - t1;
      time += ms_double.count();
      
    }
    
  }
  return time;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int Graph::bidiSP(int src, int dst) {

  int pid = -1;

  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds; 
  auto t1 = high_resolution_clock::now();
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;
  double time = 0;

  queue<int> srcq;
  queue<int> dstq;

  set<int> srcv;
  set<int> dstv;

  t1 = high_resolution_clock::now();
  pid = get(src, -2);
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  time += ms_double.count();
  
  srcq.push(src);
  srcv.insert(src);

  if (src == dst) {
    return 0;
  }

  t1 = high_resolution_clock::now();
  pid = get(dst, -2);
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  time += ms_double.count();

  dstv.insert(dst);
  dstq.push(dst);

  int level = 1; //sp-r r->level
  while (!srcq.empty() || !dstq.empty()) {
    int size = srcq.size();
    for (int j = 0; j < size; j++) {
      int curNode = srcq.front();
      srcq.pop();
      if (nodeOrder[curNode] == -1) {
        continue;
      }
      int st = nodeOrder[curNode] == 0 ? 0 : index[nodeOrder[curNode] - 1];
      int en = index[nodeOrder[curNode]];
      
      for (int i = st; i < en; i++) {
        if (srcv.find(edges[i]) == srcv.end()) {

          t1 = high_resolution_clock::now();
          pid = get(edges[i], -2); 
          t2 = high_resolution_clock::now();
          ms_double = t2 - t1;
          time += ms_double.count();

          srcq.push(edges[i]);
          srcv.insert(edges[i]);
        }
      }
    }

    size = dstq.size();
    for (int j = 0; j < size; j++) {
      int curNode = dstq.front();
      dstq.pop();
      if (nodeOrder[curNode] == -1) {
        continue;
      }
      int st = nodeOrder[curNode] == 0 ? 0 : index[nodeOrder[curNode] - 1];
      int en = index[nodeOrder[curNode]];
      
      for (int i = st; i < en; i++) {
        if (dstv.find(edges[i]) == dstv.end()) {
          
          t1 = high_resolution_clock::now();
          pid = get(edges[i], -2); 
          t2 = high_resolution_clock::now();
          ms_double = t2 - t1;
          time += ms_double.count();
         
          dstq.push(edges[i]);
          dstv.insert(edges[i]);
        }
      }
    }

    set<int> intersect;
    set_intersection(srcv.begin(), srcv.end(), dstv.begin(), dstv.end(),
                     std::inserter(intersect, intersect.begin()));

    if (intersect.size() > 1 ||
        (intersect.size() == 1 && *(intersect.begin()) != src)) {
      return time;
    }
    level++;
    if(level > maxSPLevel) break; 
  }
  
  return time;
}

long Graph::printStats() { 
  return cache->printStats(); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Graph::fr_all_2(int src) {

  int pid = -1;


  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds; 
  auto t1 = high_resolution_clock::now();
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;
  double time = 0;

  queue<int> srcq;
  queue<int> dstq;

  set<int> srcv;
  set<int> dstv;

  t1 = high_resolution_clock::now();
  pid = get(src, -2);
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  time += ms_double.count();

  srcq.push(src);
  srcv.insert(src);

  int level = 1; //sp-r r->level
  while (!srcq.empty()) {
    int size = srcq.size();
    for (int j = 0; j < size; j++) {
      int curNode = srcq.front();
      srcq.pop();
      if (nodeOrder[curNode] == -1) {
        continue;
      }
      int st = nodeOrder[curNode] == 0 ? 0 : index[nodeOrder[curNode] - 1];
      int en = index[nodeOrder[curNode]];
      
      for (int i = st; i < en; i++) {
        if (srcv.find(edges[i]) == srcv.end()) {

          t1 = high_resolution_clock::now();
          pid = get(edges[i], -2); 
          t2 = high_resolution_clock::now();
          ms_double = t2 - t1;
          time += ms_double.count();

          srcq.push(edges[i]);
          srcv.insert(edges[i]);
        }
      }
    }
    level++;
    if(level > 2) break; 
  }
  
  return time;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Graph::rw(int src, int NS, int NW){
  
  int pid = -1;
  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds; 
  auto t1 = high_resolution_clock::now();
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;
  double time = 0;

  t1 = high_resolution_clock::now();
  pid = get(src, -2);
  t2 = high_resolution_clock::now();
  ms_double = t2 - t1;
  time += ms_double.count();
  
  if (nodeOrder[src] == -1) {
    return 0;
  }

  int degree = -1;
  std::unordered_map<int, int>::iterator it = nodeDegree.find(src);
  if (it != nodeDegree.end()) {
    degree = it->second;
  }
  if(NW>degree) NW=degree;

  for(int nw=0; nw<NW; nw++){
    int currNode = src;
   
    for(int ns = 0; ns<NS; ns++){
    	if (nodeOrder[currNode] != -1) {
	      int st = nodeOrder[currNode] == 0 ? 0 : index[nodeOrder[currNode] - 1];
	      int en = index[nodeOrder[currNode]];
	      int offset = st+1;
	      int range = en-st;
	      int i = offset + (rand() % range);
	      currNode = edges[i];
		    
        t1 = high_resolution_clock::now();
        pid = get(currNode, -2); 
        t2 = high_resolution_clock::now();
        ms_double = t2 - t1;
        time += ms_double.count();
      } 
      else{
      	break;
      }   
    }
  }
  return time;
}
