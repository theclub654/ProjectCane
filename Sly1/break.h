#pragma once
#include "so.h"

enum BRPT
{
    BRPT_Nil = -1,
    BRPT_Disappear = 0,
    BRPT_Remain = 1,
    BRPT_Fixed = 2,
    BRPT_RemainFixed = 3,
    BRPT_Appear = 4,
    BRPT_Max = 5
};
struct BRP 
{
    BRPT brpt;
    OID oid;
    struct LO* plo;
};
enum EXRK 
{
    EXRK_Nil = -1,
    EXRK_Self = 0,
    EXRK_All = 1,
    EXRK_Max = 2
};
enum BRKPS
{
    BRKPS_Nil = -1,
    BRKPS_Idle = 0,
    BRKPS_Pending = 1,
    BRKPS_Fading = 2,
    BRKPS_Max = 3
};

class BRK : public SO
{
	public:
        int cbrp;
        BRP abrp[16];
        OID oidExplBreak;
        struct EXPL* pexplBreak;
        EXRK exrk;
        float rExplodeScale;
        float rvAbsorbWhack;
        LM lmBreakPosToV;
        float svBreak;
        float swBreak;
        OID oidHiPri;
        int grfbrk;
        int coidTouch;
        OID aoidTouch[4];
        int ccidTouch;
        CID acidTouch[4];
        int fBreakOnUpdate;
        int fRemain;
        int fBroken;
        int ichkBroken;
        int cHitsToBreak;
        int cHits;
        int fIgnoreBrkp;
        int ccoin;
        glm::vec3 dposCoin;
        struct SFX* psfxBreak;
};

class BREAK : public BRK
{
	public:
};

class FRAGILE : public BRK
{
	public:
        struct SO* psoImpacting;
};

class BRKP : public SO
{
    public:
        BRKPS brkps;
        float tBrkps;
        float dtMaxLifetime;
        float dtFade;
        struct BRK* pbrk;
        int fIgnoreBrkp;
};

class ZAPBREAK : public FRAGILE
{
    public:
        ZPK zpk;
        struct PO* ppoZap;
};

void* NewBrk();
void InitBrk(BRK* pbrk);
void DeleteBrk(LO* plo);
void* NewBrkp();
void DeleteBrkp(LO* plo);
void* NewBreak();
void InitBreak(BREAK* pbreak);
void DeleteBreak(LO* plo);
void* NewFragile();
void InitFragile(FRAGILE* pfragile);
void DeleteFragile(LO* plo);
void* NewZapbreak();
void DeleteZapbreak(LO* plo);
void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);