#pragma once
#include "shdanim.h"

struct PINGPONG : public SAA
{
    float dtPingpongMin;
    float dtPingpongMax;
    float dtPauseMin;
    float dtPauseMax;
    float sviframe;
    float gframe;
    float dtPauseRequested;
    float dtPause;
};

std::shared_ptr<PINGPONG> NewPingPong();
void  LoadPingPongFromBrx(PINGPONG* ppingpong, CBinaryInputStream* pbis);
void  InitPingpong(PINGPONG* ppingpong, SAAF* psaaf);
void  PostPingpongLoad(PINGPONG* ppingpong);
void  UpdatePingpong(PINGPONG* ppingpong, float dt);
float UCompletePingpong(PINGPONG* ppingpong);
void  DeletePingpong(PINGPONG* ppingpong);
