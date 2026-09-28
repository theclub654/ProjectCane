#pragma once
#include "shdanim.h"

struct LOOKER : public SAA
{
    float uCenter;
    float vCenter;
    float duMin;
    float duMax;
    float dvMin;
    float dvMax;
    int cvtx;
    struct UVQ* puvqd;
    struct POSAD* pposad;
};

std::shared_ptr<LOOKER> NewLooker();
void  LoadLookerFromBrx(LOOKER* plooker, CBinaryInputStream* pbis);
void  InitLooker(LOOKER* plooker, SAAF* psaaf);
void  NotifyLookerRender(LOOKER* plooker, ALO* palo, RPL* prpl);
void  DeleteLooker(LOOKER* plooker);
