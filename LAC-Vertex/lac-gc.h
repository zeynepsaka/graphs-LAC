
#include <vector>
#include <map>
#include <unordered_map>
#include "cache.h"
struct LACGPage{
    int pageID;
    std::vector<int> vertexList;
    float freq;
    LACGPage(int p_pageID, float p_freq):pageID(p_pageID),freq(p_freq){}
};

class LACGCCache: public Cache{
public:

    int clock;
    bool warmCache;
    int tempCapacity;
    static float maxDist;

    std::vector<LACGPage*> pageClock;
    std::map<int,int> pageIndex; //pageID -> pageClock Index
    //std::map<int,int> coldPageIndex; 

    std::map<int, int> pageDistanceMap;


    
    std::unordered_map<int,int> vertexToPageMap;


    int get(int src, int dst) override;

    void fetchPage(int pageID);
    void fetchColdPage(int pageID);

    int evictPage();
    int evictColdPage();
    float getDistance(int pageID);

    LACGCCache(Storage* storage, int capacity, int blockSize, float pageOnDisk);

};
