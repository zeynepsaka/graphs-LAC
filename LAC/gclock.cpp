#include "gclock.h"
#include <iostream>
#include <fstream>
#include <iostream>

GClockCache::GClockCache(Storage* storage, int capacity, int blockSizeVertex, int blockSizeEdge, float pageOnDisk)
    : Cache(storage, capacity, blockSizeVertex, blockSizeEdge, pageOnDisk), clock(0), warmCache(false) {
        pageClock.resize(capacity);
        for(int i = 0 ; i < capacity ; i ++){
            GPage *page = new GPage(-1*(i+1),1);
            pageClock[i] = page;
            pageIndex[page->pageID] = i;
        }
    }

int GClockCache::get(int src, int vrs){
    
        int pageID = storage->getBlockID(src, vrs);
        if(pageIndex.find(pageID) != pageIndex.end()){
            int hitClock = pageIndex[pageID];
            if(pageClock[hitClock]->freq < 4){ 
                pageClock[hitClock]->freq++;
            }
            if(warmCache){
                numHit++;
            }
                
            return pageID; 
        }
        
        int evictedPage = evictPage();
        
        if(evictedPage > 0) {
            numMiss++; 
            warmCache = true;
        }
        
        fetchPage(pageID);
        return pageID;
}

int GClockCache::evictPage(){
   
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

void GClockCache::fetchPage(int pageID){
   // storage->fetchFromStorageOffset(pageID);
    GPage *page = new GPage(pageID,1);
    pageClock[clock] = page;
    pageIndex[pageID] = clock;
    clock = (clock+1)%capacity;
}
