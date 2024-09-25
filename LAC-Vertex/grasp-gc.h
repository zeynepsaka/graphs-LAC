
#include <vector>
#include <map>
#include <unordered_set>
#include "cache.h"
struct GraspPage{
    int pageID;
    int freq;
    int level;
    GraspPage(int p_pageID, long p_freq, int p_level):pageID(p_pageID),freq(p_freq), level(p_level){}
};

class GraspGClockCache: public Cache{
public:

    int clock;
    bool warmCache;
    
    std::vector<GraspPage*> pageClock;
    std::unordered_map<int,int> pageIndex; //pageID -> pageClock Index

    int get(int src, int vrs) override;

    void fetchPage(int pageID, int level);
    int evictPage();

    GraspGClockCache(Storage* storage, int capacity, int blockSize, float pageOnDisk);

};
