#ifndef GCLOCK_CACHE_H
#define GCLOCK_CACHE_H

#include <vector>
#include <map>
#include <unordered_set>
#include "cache.h"
struct GPage{
    int pageID;
    int freq;
    GPage(int p_pageID, long p_freq):pageID(p_pageID),freq(p_freq){}
};

class GClockCache: public Cache{
public:

    int clock;
    bool warmCache;
    
    std::vector<GPage*> pageClock;
    std::unordered_map<int,int> pageIndex; //pageID -> pageClock Index

    int get(int src, int vrs) override;

    void fetchPage(int pageID);
    int evictPage();

    GClockCache(Storage* storage, int capacity, int blockSizeVertex, int blockSizeEdge, float pageOnDisk);

};

#endif //GCLOCK_CACHE_H