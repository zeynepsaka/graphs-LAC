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

//int LACGCCache::maxDist =-1;

LACGCCache::LACGCCache(Storage* storage, int capacity, int blockSizeVertex, int blockSizeEdge, float pageOnDisk)
    : Cache(storage, capacity, blockSizeVertex, blockSizeEdge, pageOnDisk), clock(0), warmCache(false), tempCapacity(0){ //log(pageOnDisk-capacity-1) 5 best time
        pageClock.resize(capacity);
        for(int i = 0 ; i < capacity ; i ++){
            LACGPage *page = new LACGPage(-1*(i+1),1);
            pageClock[i] = page;
        }
        maxDist = round(log10(pageOnDisk-capacity-1));

        diskPageIndex=std::vector<float>(pageOnDisk, 0);
        diskPageClock=std::vector<float>(pageOnDisk, -1);
        
    }

int LACGCCache::get(int src, int vrs){ //src: order in edgeIndex

    int pageID = storage->getBlockID(src, vrs);

    if(diskPageIndex[pageID]>0){
    //if(pageIndex.find(pageID) != pageIndex.end()){
        //int hitClock = pageIndex[pageID];
        int hitClock = diskPageClock[pageID];

        //float dist = getDistance(pageID);
        float dist = getDistanceDisk(pageID);
        
        if(tempCapacity>=capacity){
            numHit++;
        }
        
        if(dist>maxDist) 
            dist=maxDist;
        dist = dist/maxDist;
        dist = (1-dist);
        
        if(pageClock[hitClock]->freq + dist <= 4){
            pageClock[hitClock]->freq = pageClock[hitClock]->freq + dist;
            //faster impl:
            diskPageIndex[pageID] = pageClock[hitClock]->freq;
        }
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
    return evictPageID;
}

int LACGCCache::evictPage(){
    //float dist = getDistance(pageClock[clock]->pageID);
    float dist = getDistanceDisk(pageClock[clock]->pageID);
    
    if(dist>maxDist) 
        dist=maxDist;
    dist = dist/maxDist;
    pageClock[clock]->freq = pageClock[clock]->freq - dist;
    
    diskPageIndex[pageClock[clock]->pageID] = pageClock[clock]->freq;

    int turnAround = 0;
    bool flag = false;
    while(pageClock[clock]->freq > 0){
        clock = (clock+1)%capacity;
        if(flag || (++turnAround)%capacity == 0){
            flag = true;
            dist = 1;
        }
        else{
            //dist = getDistance(pageClock[clock]->pageID);
            dist = getDistanceDisk(pageClock[clock]->pageID);
            if(dist>maxDist) 
                dist=maxDist;
            dist = dist/maxDist;
        }
        pageClock[clock]->freq = pageClock[clock]->freq - dist;
        diskPageIndex[pageClock[clock]->pageID] = pageClock[clock]->freq;
    }
    
    int evictPageID = pageClock[clock]->pageID;
    delete pageClock[clock];
    diskPageIndex[evictPageID] = 0;
    diskPageClock[evictPageID] = -1;
    //pageIndex.erase(evictPageID);
    return evictPageID;
}


void LACGCCache::fetchPage(int pageID){
    //pageIndex[pageID] = clock;
    //float dist = getDistance(pageID);
    
    // float dist = getDistanceDiskEdge(pageID);
    // if(dist>maxDist) 
    //     dist=maxDist;
    // dist = dist/maxDist;
    // dist = (1-dist);

    float dist = 1;

    //storage->fetchFromStorageOffset(pageID);

    LACGPage *page = new LACGPage(pageID,dist); //dist
    pageClock[clock] = page;
    //pageIndex[pageID] = clock;
    diskPageClock[pageID] = clock;
    diskPageIndex[pageID] = dist;
    clock = (clock+1)%capacity;
}

// float LACGCCache::getDistance(int pageID){
//     std::map<int,int>::iterator iter = pageIndex.find(pageID);
//     int leftDist = iter->first;
//     int rightDist = iter->first;
//     if (iter == pageIndex.begin()) {
//         leftDist = INT_MAX;
//     } else {
//         //if(std::prev(iter)->first >= 0)
//             leftDist = iter->first - std::prev(iter)->first;
//             if(leftDist==1) return leftDist; //no need to check right distance
//     }
//     auto nextIter = std::next(iter);
//     if (nextIter == pageIndex.end()) {
//         rightDist = INT_MAX;
//     } else {
//         //if(nextIter->first >= 0)
//             rightDist = nextIter->first - iter->first;
//     }
//     if(leftDist == INT_MAX && rightDist == INT_MAX) return 0;
//     return (float) std::min(leftDist, rightDist);
// }

// float LACGCCache::getDistanceDiskVertex(int pageID){
//     int leftDist = INT16_MAX;
//     int rightDist = INT16_MAX;

//     int curr = pageID;

//     // Right search: Find the nearest non-zero on the right
//     int i = curr + 1;
//     int maxI = std::min((int)diskPageIndexVertex.size() - 1, curr + maxDist);
//     while (i <= maxI) {
//         if (diskPageIndexVertex[i] != 0) {
//             rightDist = i - pageID;
//             break; // Exit as soon as we find the nearest non-zero on the right
//         }
//         i++;
//     }

//     // Left search: Find the nearest non-zero on the left
//     i = curr - 1;
//     int minI = std::max(0, curr - maxDist);
//     while (i >= minI) {
//         if (diskPageIndexVertex[i] != 0) {
//             leftDist = pageID - i;
//             break; // Exit as soon as we find the nearest non-zero on the left
//         }
//         i--;
//     }

//     // Return the smallest distance or MAX_DIST
//     return std::min(std::min(leftDist, rightDist), maxDist);
// }

float LACGCCache::getDistanceDisk(int pageID){
    int leftDist = INT16_MAX;
    int rightDist = INT16_MAX;

    int curr = pageID;

    // Right search: Find the nearest non-zero on the right
    int i = curr + 1;
    int maxI = std::min((int)diskPageIndex.size() - 1, curr + maxDist);
    while (i <= maxI) {
        if (diskPageIndex[i] != 0) {
            rightDist = i - pageID;
            break; // Exit as soon as we find the nearest non-zero on the right
        }
        i++;
    }

    // Left search: Find the nearest non-zero on the left
    i = curr - 1;
    int minI = std::max(0, curr - maxDist);
    while (i >= minI) {
        if (diskPageIndex[i] != 0) {
            leftDist = pageID - i;
            break; // Exit as soon as we find the nearest non-zero on the left
        }
        i--;
    }

    // Return the smallest distance or MAX_DIST
    return std::min(std::min(leftDist, rightDist), maxDist);
}
