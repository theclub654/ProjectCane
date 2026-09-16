#pragma once
#include "po.h"

struct MGCO
{
    class ALO* palo;
    struct ASEGA* pasegaPath;
    int fGood;
    DLE dle;
};

struct TMBL 
{
    class ALO* palo;
    struct SM* psmDial;
    struct SMA* psmaDial;
};

class MGC : public ALO
{
	public:
    struct SM* psmGenerate;
    struct SMA* psmaGenerate;
    struct SM* psmCollect;
    struct SMA* psmaCollect;
    struct ASEG* pasegPath;
    struct VOL* pvolSmash;
    struct VOL* pvolHole;
    DL dlMgcoFree;
    DL dlMgco;
    int fDamaging;
    struct EXPL* pexplGood;
    struct EXPL* pexplBad;
    struct MGV* pmgv;
};

MGC* NewMgc();
void InitMgc(MGC* pmgc);
int  GetMgcSize();
void CloneMgc(MGC* pmgc, MGC* pmgcBase);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
void HandleMgvMessage(MGV* pmgv, MSGID msgid, void* pv);
void DeleteMgc(MGC* pmgc);