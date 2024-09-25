#include "grasp-gc.h"
#include <iostream>
#include <fstream>
#include <iostream>

GraspGClockCache::GraspGClockCache(Storage* storage, int capacity, int blockSize, float pageOnDisk)
    : Cache(storage, capacity, blockSize, pageOnDisk), clock(0), warmCache(false) {
        pageClock.resize(capacity);
        for(int i = 0 ; i < capacity ; i ++){
            GraspPage *page = new GraspPage(-1*(i+1),1,-1); //level is -1 because this data is temprorary until it is filled with an actual page
            pageClock[i] = page;
            pageIndex[page->pageID] = i;
        }
    }

int GraspGClockCache::get(int src, int vrs){ 

        int pageID = storage->getBlockID(src, vrs);
        int level = storage->getLevel(pageID);
        if(pageIndex.find(pageID) != pageIndex.end()){
            //HIT
            int hitClock = pageIndex[pageID];
            

            pageClock[hitClock]->freq = level; //hit promotion
            pageClock[hitClock]->level = level; //no need for this
            if(warmCache){ 
                numHit++; //only warm cache
            }
            
            return pageID; 
        }
        
        int evictedPage = evictPage();
        
        if(evictedPage > 0) {
            numMiss++; 
            warmCache = true;
        }
        
        fetchPage(pageID, level); //fetch insertion promotion
        return pageID;

}

int GraspGClockCache::evictPage(){
   
    int tempClock = 0;
    while(--(pageClock[clock]->freq) != 0){
        tempClock++;
        //if(tempClock>pageClock.size()) printf("more than one tour\n");
        clock = (clock+1)%capacity;
    }
    tempClock = 0;
    int evictPageID = pageClock[clock]->pageID;
    delete pageClock[clock];
    pageIndex.erase(evictPageID);
    return evictPageID;
}

void GraspGClockCache::fetchPage(int pageID, int level){
    //storage->fetchFromStorageOffset(pageID);
    
    GraspPage *page = new GraspPage(pageID,level,level); //freq equals level (pretending we decide on the position (MRU, LRU, etc..))
    pageClock[clock] = page;
    pageIndex[pageID] = clock;
    clock = (clock+1)%capacity;
}
