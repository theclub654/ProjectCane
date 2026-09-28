#pragma once
#include "shdanim.h"

struct CIRCLER : public SAA
{
    float sw;
    float sRadius;
    float du;
    float dv;
};

std::shared_ptr<CIRCLER> NewCircler();
void  LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis);
void  InitCircler(CIRCLER* pcircler, SAAF* psaaf);
void  UpdateCircler(CIRCLER* pcircler, float dt);
float UCompleteCircler(CIRCLER* pcircler);
void  DeleteCircler(CIRCLER* pcircler);
