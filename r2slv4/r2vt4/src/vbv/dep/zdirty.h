//
//  zdirty.h
//  r2vt4
//
//  Created by hadoop on 5/15/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zdirty__
#define __r2vt4__zdirty__

#include <time.h>
#include "../../zmath.h"

namespace __r2vt4__ {
  
  /** AE Learning solution (audio envionment learning) for environment detecting and filtering
   @author Misa.Z (misa@rokid.com) 2016.06.
   */
  
  
  //Audio input sector for each sector in 360-degree
  struct AudioInputSector {
    unsigned int dirty; //times of invalid VAD.
    time_t tryTime; 			//last time trying to set this sector available.
    time_t dirtyTime;		//last time invalid VAD.
    time_t dirtySuccessTime ; //last success time.
    
    AudioInputSector() {
      dirty = 0;
      tryTime = 0;
      dirtyTime = 0;
      dirtySuccessTime = 0;
    }
  };
  
  
#define SECTOR_NUM	72 // number of sectors, that make 10-degree each sector
#define SECTOR_TOR  4
  
  class ZDirty {
    
  public:
    /**
     @param dMax, max dirty times for get guity.
     @param dTimeOut, dirty time out for set sector free.
     @param tTimeOut, try time out for ignore second trying to set sector free.
     */
    ZDirty(bool bD, unsigned int dMax, time_t tDirtyTimeOut, time_t tTryTimeOut, time_t tDirtySuccessTimeOut);
    
  public:
    ~ZDirty(void);
    
  protected:
    
    AudioInputSector* sectors;
    unsigned int dirtyMax;
    time_t dirtyTimeOut;
    time_t tryTimeOut;
    time_t dirtySuccessTimeOut ;
    bool bAffect ;
    
    
    
  public:
    
    /**
     Mark the specific sector available.
     @param index, the index of sectors.
     */
    void setFree(int index);
    void setFree2(int index);

    void reset();
    /**
     check if the specific sector available or not, will add one more trying count.
     @param index, the index of sectors.
     @return true, if available.
     */
    bool check(int index);
    
    /**
     get sector index by degree.
     @param degree, the specific degree.
     @return index, sector index.
     */
    static int getIndexByDegree(int degree);
    
    /**
     Mark the specific sector dirty, when got a invalid VAD.
     @param index, the index of sectors.
     */
    void dirty(int index);
    void dirty2(int index);
    
  };
  
};


#endif /* __r2vt4__zdirty__ */
