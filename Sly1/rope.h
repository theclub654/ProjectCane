#pragma once
#include "alo.h"

enum ROPEK 
{
    ROPEK_Nil = -1,
    ROPEK_FixedLength = 0,
    ROPEK_FixedSlack = 1,
    ROPEK_FixedCurve = 2,
    ROPEK_Max = 3
};

class ROPE : public ALO
{
	public:
        ROPEK ropek;
        OID oidOther;
        struct PNT* ppntOther;
        OID oidShd;
        struct SHD* pshd;
        float sLength;
        float dsSlack;
        float r;
        int cLength;
        int cAxis;
        float sRadius;
        float vMax;
        CLQ clqSToUMax;
};

void*NewRope();
void InitRope(ROPE* prope);
int  GetRopeSize();
void CloneRope(ROPE* prope, ROPE* propeBase);
void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro);