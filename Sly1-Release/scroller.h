#pragma once
#include "shdanim.h"

struct SCROLLER : public SAA
{
    float svu;
    float svv;
    float duMod;
    float dvMod;
    float svuMaster;
    float svvMaster;
};

SCROLLER* NewScroller();
void  LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis);
void  SetScrollerMasterSpeeds(SCROLLER* pscroller, float svu, float svv);
void  InitScroller(SCROLLER* pscroller, SAAF* psaaf);
void  UpdateScroller(SCROLLER* pscroller, float dt);
float UCompleteScroller(SCROLLER* pscroller);
void  DeleteScroller(SCROLLER* pscroller);