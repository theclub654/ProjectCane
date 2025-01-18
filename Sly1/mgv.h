#pragma once
#include "po.h"

struct TMBL 
{
    struct ALO* palo;
    struct SM* psmDial;
    struct SMA* psmaDial;
};

class MGV :public PO
{
	public:
        struct SM* psm;
        struct SMA* psma;
        struct MGC* apmgc[3];
        int ipmgcCur;
        TMBL atmblPoints[3];
        TMBL tmblLives;
        int clives;
        int cpoints;
};

MGV* NewMgv();
void InitMgv(MGV* pmgv);
int  GetMgvSize();
void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis);
void CloneMgv(MGV* pmgv, MGV* pmgvBase);
void DeleteMgv(MGV* pmgv);