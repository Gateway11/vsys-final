//
//  r2_soundlocalizer.cpp
//  SSPTest
//
//  Created by GaoPeng on 15/6/15.
//  Copyright (c) 2015年 Rokid. All rights reserved.
//

#include <algorithm>
//#include <stdlib.h>
#include <math.h>
#include "r2_soundtracker.h"


const int kMaxQueueLen = 7;


R2SoundTracker::R2SoundTracker() {
}

// return angle cos of the two directions
float CalcSoundAngle(const R2SoundCandidate &first,
                        const R2SoundCandidate &second) {
    float x1 = cos(first.azimuth) * cos(first.elevation);
    float y1 = sin(first.azimuth) * cos(first.elevation);
    float z1 = sin(first.elevation);
    float x2 = cos(second.azimuth) * cos(second.elevation);
    float y2 = sin(second.azimuth) * cos(second.elevation);
    float z2 = sin(second.elevation);
    return (x1*x2+y1*y2+z1*z2); // cos(angle)
}

void R2SoundTracker::AddCandidate(R2SoundCandidate &candi) {
    const float kMinClusterAngleCos = 0.96592582629f; // 15 degree
    
    //candi.elevation = 0; // ignore elevation
    
    // find the closest candidate
    float maxWeight = -1; // cos is larger, angle is smaller
    int maxWeightIdx = -1;
    for (int i=0; i<m_candidates.size(); i++) {
        R2SoundCandidate &sc = m_candidates[i];
        float angle = CalcSoundAngle(candi, sc);
        if (angle < kMinClusterAngleCos)
            continue;
        float weight = angle;
        if (weight > maxWeight) {
            maxWeight = weight;
            maxWeightIdx = i;
        }
    }
    
    if (maxWeightIdx == -1) { // new candidate
        candi.serial = m_serial++;
        
        m_candiqueues.resize(m_candiqueues.size()+1);
        std::vector<R2SoundCandidate> &candiqueue = m_candiqueues.back();
        candiqueue.push_back(candi);
        
        m_candidates.resize(m_candidates.size()+1);
        R2SoundCandidate &sc = m_candidates.back();
        sc = candi;
        sc.confidence /= kMaxQueueLen;
        
        m_tickupdates.resize(m_tickupdates.size()+1);
        m_tickupdates.back() = true;
    }
    else {
        std::vector<R2SoundCandidate> &candiqueue = m_candiqueues[maxWeightIdx];
        R2SoundCandidate &prevcandi = candiqueue.back();
        candi.serial = prevcandi.serial;
        if (m_tickupdates[maxWeightIdx]) { // already has a candidate during this tick
            if (candi.confidence > prevcandi.confidence) {
                prevcandi = candi;
            }
        }
        else {
            candiqueue.push_back(candi);
            m_tickupdates[maxWeightIdx] = true;
        }
    }
}

float inline GetRoundAzimuth(float azimuth, float center) {
    float a1 = azimuth - (M_PI*2);
    float a2 = azimuth;
    float a3 = azimuth + (M_PI*2);
    float d1 = fabsf(a1-center);
    float d2 = fabsf(a2-center);
    float d3 = fabsf(a3-center);
    if (d1 <= d2 && d1 <= d3)
        return a1;
    else if (d2 <= d1 && d2 <= d3)
        return a2;
    else
        return a3;
}

void R2SoundTracker::Tick() {
    for (int i=0; i<m_candiqueues.size(); i++) {
        std::vector<R2SoundCandidate> &candiqueue = m_candiqueues[i];
        if (m_tickupdates[i]) {
            m_tickupdates[i] = false;
        }
        else { // no candidate in this tick, add a silence
            R2SoundCandidate    sc;
            sc.silence = true;
            sc.serial = candiqueue.back().serial;
            candiqueue.push_back(sc);
        }
        if (candiqueue.size() > kMaxQueueLen)
            candiqueue.erase(candiqueue.begin());
        
        // update candidate
        int nosilNum = 0;
        R2SoundCandidate &sc = m_candidates[i];
        float aziCenter = 0;
        for (int j=0; j<candiqueue.size(); j++) {
            R2SoundCandidate &c = candiqueue[j];
            if (c.silence)
                continue;
            if (nosilNum == 0) { // first no silence candidate
                sc = c;
            }
            else {
                float w = sc.confidence + c.confidence * (j+1);
                float w1 = sc.confidence / w;
                float w2 = c.confidence * (j+1) / w;
                sc.azimuth = sc.azimuth * w1 + GetRoundAzimuth(c.azimuth,aziCenter) * w2;
                if (sc.azimuth < 0)
                    sc.azimuth += (M_PI*2);
                else if (sc.azimuth >= (M_PI*2))
                    sc.azimuth -= (M_PI*2);
                sc.elevation = sc.elevation * w1 + c.elevation * w2;
                sc.confidence += c.confidence * (j+1);
            }
            aziCenter = sc.azimuth;
            nosilNum ++;
        }
        if (nosilNum == 0) { // remove this candidate
            m_candiqueues.erase(m_candiqueues.begin()+i);
            m_candidates.erase(m_candidates.begin()+i);
            m_tickupdates.erase(m_tickupdates.begin()+i);
            i --;
        }
        else {
            //sc.confidence /= candiqueue.size();
            //sc.confidence *= candiqueue.size() / kMaxQueueLen;
            //sc.confidence /= kMaxQueueLen;
            int n = (int)candiqueue.size();
            sc.confidence /= n * (n+1) / 2;
            sc.ticks = nosilNum;
        }
    }
}

static inline bool CompareSoundCandidate(const R2SoundCandidate &first,
                                         const R2SoundCandidate &second) {
    return first.confidence > second.confidence;
}

int R2SoundTracker::GetCandidateNum() {
    return (int)m_candidates.size();
}

void R2SoundTracker::GetCandidates(std::vector<R2SoundCandidate> &candis) {
    candis.clear();
    candis.insert(candis.begin(), m_candidates.begin(), m_candidates.end());
    std::sort(candis.begin(), candis.end(), CompareSoundCandidate);
}

void R2SoundTracker::Reset() {
    m_candiqueues.clear();
    m_tickupdates.clear();
    m_candidates.clear();
    m_serial = 0;
}
