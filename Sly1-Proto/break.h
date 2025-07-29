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

struct BRP
{
    BRPT brpt;
    OID oid;
    struct LO* plo;
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
        SO* psoImpacting;
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

BRK* NewBrk();
void InitBrk(BRK* pbrk);
int  GetBrkSize();
void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);
void CloneBrk(BRK* pbrk, BRK* pbrkBase);
void DeleteBrk(BRK* pbrk);

BRKP*NewBrkp();
int  GetBrkpSize();
void CloneBrkp(BRKP* prkp, BRKP* prkpBase);
void DeleteBrkp(BRKP* pbrkp);

BREAK*NewBreak();
void InitBreak(BREAK* pbreak);
int  GetBreakSize();
void CloneBreak(BREAK* pbreak, BREAK* pbreakBase);
void PostBrkLoad(BRK* pbrk);
void DeleteBreak(BREAK* pbreak);

FRAGILE*NewFragile();
void InitFragile(FRAGILE* pfragile);
int  GetFragileSize();
void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase);
void DeleteFragile(FRAGILE* pfragile);

ZAPBREAK*NewZapbreak();
int  GetZapbreakSize();
void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase);
void DeleteZapbreak(ZAPBREAK* pzapbreak);