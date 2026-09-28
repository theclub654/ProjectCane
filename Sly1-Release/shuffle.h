#pragma once
#include "shdanim.h"

struct SHUFFLE : public SAA
{
    float dtPauseMin;
    float dtPauseMax;
    float dtPause;
};

std::shared_ptr<SHUFFLE> NewShuffle();
void  LoadShuffleFromBrx(SHUFFLE* pshuffle, CBinaryInputStream* pbis);
void  InitShuffle(SHUFFLE* pshuffle, SAAF* psaaf);
void  UpdateShuffle(SHUFFLE* pshuffle, float dt);
void  DeleteShuffle(SHUFFLE* pshuffle);
