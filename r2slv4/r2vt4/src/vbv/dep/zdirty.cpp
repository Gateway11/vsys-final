//
//  zdirty.cpp
//  r2vt4
//
//  Created by hadoop on 5/15/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zdirty.h"

namespace __r2vt4__ {
  
  ZDirty::ZDirty(bool bD, unsigned int dMax, time_t tDirtyTimeOut, time_t tTryTimeOut, time_t tDirtySuccessTimeOut){
    
    sectors = Z_SAFE_NEW_AR1(sectors, AudioInputSector, SECTOR_NUM) ;
    
    this->bAffect = bD ;
    
    dirtyMax = dMax;
    dirtyTimeOut = tDirtyTimeOut;
    tryTimeOut = tTryTimeOut;
    dirtySuccessTimeOut = tDirtySuccessTimeOut ;
  }
  
  ZDirty::~ZDirty(void){
    
    Z_SAFE_DEL_AR1(sectors) ;
  }
  
  
  void ZDirty::setFree(int index) {
    
    assert(index >= 0 && index < SECTOR_NUM) ;
    
    sectors[index].dirtyTime = 0;
    sectors[index].dirty = 0;
    sectors[index].tryTime = 0;
    
    time_t now = time(NULL);
    if (now < sectors[index].dirtySuccessTime) {
      if (now + dirtySuccessTimeOut * 5 > sectors[index].dirtySuccessTime) {
        sectors[index].dirtySuccessTime += dirtySuccessTimeOut ;
      }
    }else{
      sectors[index].dirtySuccessTime = now + dirtySuccessTimeOut ;
    }
  }
  
  void ZDirty::setFree2(int index){
    
    assert(index >= 0 && index < SECTOR_NUM) ;
    
    for (int i = - SECTOR_TOR ; i <= SECTOR_TOR; i ++) {
      int subindex = index + i ;
      if (subindex < 0) {
        subindex += SECTOR_NUM ;
      }
      if (subindex >= SECTOR_NUM) {
        subindex -= SECTOR_NUM ;
      }
      setFree(subindex) ;
    }
    
  }
  
  
  int ZDirty::getIndexByDegree(int degree) {
    
    assert(degree >= 0 && degree <= 360);
    
    int index = degree / (360 / SECTOR_NUM);
    
    if(index >= SECTOR_NUM)
      index = 0;
    
    return index;
  }
  
  bool ZDirty::check(int index) {
    
    if (!bAffect) {
      return  true ;
    }
    
    assert(index >= 0 && index < SECTOR_NUM) ;
    
    if(index >= SECTOR_NUM) {
      return false; // invalid sector index
    }
    
    time_t now = time(NULL);
    if (now < sectors[index].dirtySuccessTime) {
      setFree2(index);
      return true;
    }
    
    //check guity or not
    if(sectors[index].dirty < dirtyMax) { // not dirty enough to be guity
      setFree2(index);
      return true;
    }
    
    
    if((now - sectors[index].dirtyTime) >= dirtyTimeOut) { //dirty time out
      setFree2(index);
      return true;
    }
    
    if((now - sectors[index].tryTime) < tryTimeOut) { //second time try to set free.
      setFree2(index);
      return true;
    }
    
    //retry time out
    for (int i = - SECTOR_TOR ; i <= SECTOR_TOR; i ++) {
      int subindex = index + i ;
      if (subindex < 0) {
        subindex += SECTOR_NUM ;
      }
      if (subindex >= SECTOR_NUM) {
        subindex -= SECTOR_NUM ;
      }
      sectors[subindex].tryTime = now;
    }
    
    return false;
  }
  
  
  void ZDirty::dirty(int index) {
    
    assert(index >= 0 && index < SECTOR_NUM) ;
    
    if(index >= SECTOR_NUM) {
      return; // invalid sector index
    }
    
    time_t now = time(NULL);
    
    if((now - sectors[index].dirtyTime) >= dirtyTimeOut) { //dirty time out
      setFree(index);
    }
    
    sectors[index].dirty ++;
    sectors[index].dirtyTime = now;
  }
  
  void ZDirty::dirty2(int index){
    
    assert(index >= 0 && index < SECTOR_NUM) ;
    
    for (int i = -SECTOR_TOR ; i <= SECTOR_TOR; i ++) {
      int subindex = index + i ;
      if (subindex < 0) {
        subindex += SECTOR_NUM ;
      }
      if (subindex >= SECTOR_NUM) {
        subindex -= SECTOR_NUM ;
      }
      dirty(subindex) ;
    }
    
  }
  
  void ZDirty::reset() {
    for(int i=0; i<SECTOR_NUM; ++i) {
      setFree(i);
    }
  }
  
};




