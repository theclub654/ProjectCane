#pragma once
#include "shdanim.h"

struct LOOP : public SAA
{
    float dtLoopMin;
    float dtLoopMax;
    float dtPauseMin;
    float dtPauseMax;
    float sviframe;
    float gframe;
    float dtPauseRequested;
    float dtPause;
};

std::shared_ptr<LOOP> NewLoop();
void  LoadLoopFromBrx(LOOP* ploop, CBinaryInputStream* pbis);
void  InitLoop(LOOP* ploop, SAAF* psaaf);
void  PostLoopLoad(LOOP* ploop);
void  UpdateLoop(LOOP* ploop, float dt);
float UCompleteLoop(LOOP* ploop);
void  DeleteLoop(LOOP* ploop);
