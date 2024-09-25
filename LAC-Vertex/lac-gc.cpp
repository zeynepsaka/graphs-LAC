#include "lac-gc.h"

#include "storage.h"
#include <algorithm>
#include <climits>
#include <iterator>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iostream>
#include <cmath>

float LACGCCache::maxDist =-1;

LACGCCache::LACGCCache(Storage* storage, int capacity, int blockSize, float pageOnDisk)
    : Cache(storage, capacity, blockSize, pageOnDisk), clock(0), warmCache(false), tempCapacity(0){
        pageClock.resize(capacity);
        for(int i = 0 ; i < capacity ; i ++){
            LACGPage *page = new LACGPage(-1*(i+1),1);
            pageClock[i] = page;
        }

        maxDist = round(log10(pageOnDisk-capacity-1));
        
    }

int LACGCCache::get(int src, int vrs){ //src: order in edgeIndex

    int pageID = storage->getBlockID(src, vrs);

    if(pageIndex.find(pageID) != pageIndex.end()){
        int hitClock = pageIndex[pageID];
        float dist = getDistance(pageID);
        
        if(tempCapacity>=capacity){
            numHit++;
        }
        
        if(dist>maxDist) 
            dist=maxDist;
        dist = dist/maxDist;
        dist = (1-dist);
        
        if(pageClock[hitClock]->freq + dist <= 4)
            pageClock[hitClock]->freq = pageClock[hitClock]->freq + dist;
        return pageID; 
    }
    
    if(tempCapacity<capacity){
        tempCapacity++;
        evictColdPage();
        fetchPage(pageID);
    }
    else{
        int evictedPage = evictPage();
        fetchPage(pageID);
        numMiss++; //only warm cache
    }
    
    return pageID;
    
}

int LACGCCache::evictColdPage(){
    while(--(pageClock[clock]->freq) != 0){
        clock = (clock+1)%capacity;
    }
    int evictPageID = pageClock[clock]->pageID;
    delete pageClock[clock];
    //pageIndex.erase(evictPageID);
    return evictPageID;
}

int LACGCCache::evictPage(){
    float dist = getDistance(pageClock[clock]->pageID);
    
    if(dist>maxDist) 
        dist=maxDist;
    dist = dist/maxDist;
    pageClock[clock]->freq = pageClock[clock]->freq - dist;

    int turnAround = 0;
    bool flag = false;
    while(pageClock[clock]->freq > 0){
        clock = (clock+1)%capacity;
        if(flag || (++turnAround)%capacity == 0){
            flag = true;
            dist = 1;
        }
        else{
            dist = getDistance(pageClock[clock]->pageID);
            if(dist>maxDist) 
                dist=maxDist;
            dist = dist/maxDist;
        }
        pageClock[clock]->freq = pageClock[clock]->freq - dist;
    }
    
    
    int evictPageID = pageClock[clock]->pageID;
    delete pageClock[clock];
    pageIndex.erase(evictPageID);
    //storage->pageIsInCache.find(evictPageID)->second = false;
    return evictPageID;
}

void LACGCCache::fetchPage(int pageID){
    pageIndex[pageID] = clock;
    float dist = getDistance(pageID);
    
    //dist = log(dist);
    //dist = dist/maxDist;
    if(dist>maxDist) 
        dist=maxDist;
    dist = dist/maxDist;
    dist = (1-dist);

    LACGPage *page = new LACGPage(pageID,dist);
    pageClock[clock] = page;
    //storage->pageIsInCache.find(pageID)->second = true;
    pageIndex[pageID] = clock;
    clock = (clock+1)%capacity;
}

float LACGCCache::getDistance(int pageID){
    std::map<int,int>::iterator iter = pageIndex.find(pageID);
    int leftDist = iter->first;
    int rightDist = iter->first;
    if (iter == pageIndex.begin()) {
        leftDist = INT_MAX;
    } else {
        //if(std::prev(iter)->first >= 0)
            leftDist = iter->first - std::prev(iter)->first;
            if(leftDist==1) return leftDist;
    }
    auto nextIter = std::next(iter);
    if (nextIter == pageIndex.end()) {
        rightDist = INT_MAX;
    } else {
        //if(nextIter->first >= 0)
            rightDist = nextIter->first - iter->first;
    }
    if(leftDist == INT_MAX && rightDist == INT_MAX) return 0;
    return (float) std::min(leftDist, rightDist);
}
