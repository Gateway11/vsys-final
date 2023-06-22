//
//  r2_soundtracker.h
//  SSPTest
//
//  Created by GaoPeng on 15/6/15.
//  Copyright (c) 2015年 Rokid. All rights reserved.
//

#ifndef __r2ssp_soundtracker__
#define __r2ssp_soundtracker__

#include <vector>


struct R2SoundCandidate {
    int serial;
    float azimuth;
    float elevation;
    float confidence;
    int ticks;
    bool silence;
    
    R2SoundCandidate() {
        serial = 0;
        azimuth = 0;
        elevation = 0;
        confidence = 0;
        ticks = 0;
        silence = false;
    }
};


class R2SoundTracker {
public:
    R2SoundTracker();
    
    void AddCandidate(R2SoundCandidate &candi);
    void Tick();
    int GetCandidateNum();
    void GetCandidates(std::vector<R2SoundCandidate> &candis);
    void Reset();
    
private:
    std::vector<std::vector<R2SoundCandidate> > m_candiqueues;
    std::vector<bool> m_tickupdates;
    std::vector<R2SoundCandidate> m_candidates;
    int m_serial;
};

#endif /* defined(__r2ssp_soundtracker__) */
