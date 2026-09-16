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

struct MSGBRKP
{
    BRK* pbrk;
    BRKP* pbrkp;
};

class BRK : public SO
{
	public:
    int cbrp;
    BRP abrp[16];
    short oidExplBreak;
    struct EXPL* pexplBreak;
    EXRK exrk;
    float rExplodeScale;
    float rvAbsorbWhack;
    LM lmBreakPosToV;
    float svBreak;
    float swBreak;
    short oidHiPri;
    int grfbrk;
    int coidTouch;
    OID aoidTouch[4];
    int ccidTouch;
    CID acidTouch[4];
    int ccidAbsorb;
    CID acidAbsorb[3];
    int fBreakOnUpdate;
    int fRemain;
    int fBroken;
    int ichkBroken;
    int cHitsToBreak;
    int cHits;
    int fIgnoreBrkp;
    int ccoin;
    glm::vec3 dposCoin;
    std::shared_ptr <SFX> psfxBreak;
    int fSkipBreakEffects;
    int fSuppressDifficultyUpdate;
};

class BREAK : public BRK
{
	public:
};

class FRAGILE : public BRK
{
	public:
    float rImpactMass;
    CNSTR cnstrImpactForce;
    CNSTR cnstrImpactTorque;
    class SO* psoImpacting;
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
void SetBrkFRemain(BRK* pbrk, int fRemain);
void*GetBrkFRemain(BRK* pbrk);
void SetBrkRvAbsorbWhack(BRK* pbrk, float rvAbsorbWhack);
void*GetBrkRvAbsorbWhack(BRK* pbrk);
void SetBrkOidHiPri(BRK* pbrk, OID oidHiPri);
void*GetBrkOidHiPri(BRK* pbrk);
void SetBrkLmBreakPosToV(BRK* pbrk, const LM* plmBreakPosToV);
void*GetBrkLmBreakPosToV(BRK* pbrk);
void SetBrkSvBreak(BRK* pbrk, float svBreak);
void*GetBrkSvBreak(BRK* pbrk);
void SetBrkSwBreak(BRK* pbrk, float swBreak);
void*GetBrkSwBreak(BRK* pbrk);
void SetBrkCHitsToBreak(BRK* pbrk, int cHitsToBreak);
void*GetBrkCHitsToBreak(BRK* pbrk);
void*GetBrkBroken(BRK* pbrk);
void SetBrkFIgnoreBrkp(BRK* pbrk, int fIgnoreBrkp);
void*GetBrkFIgnoreBrkp(BRK* pbrk);
void SetBrkExrk(BRK* pbrk, int exrk);
void*GetBrkExrk(BRK* pbrk);
void SetBrkRExplodeScale(BRK* pbrk, float rExplodeScale);
void*GetBrkRExplodeScale(BRK* pbrk);
void SetBrkCcoin(BRK* pbrk, int ccoin);
void*GetBrkCcoin(BRK* pbrk);
void SetBrkDposCoin(BRK* pbrk, glm::vec3 dposCoin);
void*GetBrkDposCoin(BRK* pbrk);
void SetBrkFSuppressDifficultyUpdate(BRK* pbrk, int fSuppressDifficultyUpdate);
void*GetBrkFSuppressDifficultyUpdate(BRK* pbrk);
SFXID*PsfxEnsureBrksfxid(BRK* pbrk);
float*PsfxEnsureBrksStart(BRK* pbrk);
float*PsfxEnsureBrksFull(BRK* pbrk);
float*PsfxEnsureBrkuVol(BRK* pbrk);
void SetBrkOidExplBreak(BRK* pbrk, OID oidExplBreak);
void*GetBrkOidExplBreak(BRK* pbrk);
void SetBrkFSkipBreakEffects(BRK* pbrk, int fSkipBreakEffects);
void*GetBrkFSkipBreakEffects(BRK* pbrk);
void CloneBrk(BRK* pbrk, BRK* pbrkBase);
void PostBrkLoad(BRK* pbrk);
void PostBrkLoadCallbackHookup(BRK* pbrk, MSGID msgid, void* pv);
void UpdateBrk(BRK* pbrk, float dt);
int  FAbsorbBrkWkr(BRK* pbrk, WKR* pwkr);
void BreakBrk(BRK* pbrk);
void SetBrkBroken(BRK* pbrk, int fBroken);
void SetBrkExclude(BRK* pbrk, OID oid);
void SetBrkRemain(BRK* pbrk, OID oid);
void SetBrkFixed(BRK* pbrk, OID oid);
void SetBrkRemainFixed(BRK* pbrk, OID oid);
void SetBrkOnPlayer(BRK* pbrk, int fBreakOnPlayer);
void GetBrkOnPlayer(BRK* pbrk, int* pfBreakOnPlayer);
void SetBrkOnBomb(BRK* pbrk, int fBreakOnBomb);
void GetBrkOnBomb(BRK* pbrk, int* pfBreakOnBomb);
void AddBrkTouchObject(BRK* pbrk, OID oid);
void AddBrkTouchClass(BRK* pbrk, CID cid);
void AddBrkAbsorbClass(BRK* pbrk, CID cid);
SFX* PsfxEnsureBrk(BRK* pbrk, ENSK ensk);
int  FCheckBrkTouchObject(BRK* pbrk, SO* psoRoot);
void DeleteBrk(BRK* pbrk);

BRKP*NewBrkp();
void SetBrkpDtMaxLifetime(BRKP* pbrkp, float dtMaxLifetime);
void*GetBrkpDtMaxLifetime(BRKP* pbrkp);
void SetBrkpDtFade(BRKP* pbrkp, float dtFade);
void*GetBrkpDtFade(BRKP* pbrkp);
void SetBrkpFIgnoreBrkp(BRKP* pbrkp, int fIgnoreBrkp);
void*GetBrkpFIgnoreBrkp(BRKP* pbrkp);
int  GetBrkpSize();
void CloneBrkp(BRKP* prkp, BRKP* prkpBase);
void HandleBrkpMessage(BRKP* pbrkp, MSGID msgid, void* pv);
int  FIgnoreBrkpIntersection(BRKP* pbrkp, SO* psoOther);
void UpdateBrkp(BRKP* pbrkp, float dt); 
void SetBrkpBrkps(BRKP* pbrkp, BRKPS brkps);
void*GetBrkpBrkps(BRKP* pbrkp);
void DeleteBrkp(BRKP* pbrkp);

BREAK*NewBreak();
void InitBreak(BREAK* pbreak);
int  GetBreakSize();
void CloneBreak(BREAK* pbreak, BREAK* pbreakBase);
void DeleteBreak(BREAK* pbreak);

FRAGILE*NewFragile();
void InitFragile(FRAGILE* pfragile);
void SetFragileRImpactMass(FRAGILE* pfragile, float rImpactMass);
void*GetFragileRImpactMass(FRAGILE* pfragile);
void SetFragileCnstrImpactForce(FRAGILE* pfragile, CNSTR cnstrImpactForce);
void*GetFragileCnstrImpactForce(FRAGILE* pfragile);
void SetFragileCnstrImpactTorque(FRAGILE* pfragile, CNSTR cnstrImpactTorque);
void*GetFragileCnstrImpactTorque(FRAGILE* pfragile);
void SetFragilePsoImpacting(FRAGILE* pfragile, SO* psoImpacting);
void*GetFragilePsoImpacting(FRAGILE* pfragile);
void AdjustFragileNewXp(FRAGILE* pfragile, XP* pxp, int ixpd);
int  GetFragileSize();
void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase);
void DeleteFragile(FRAGILE* pfragile);

ZAPBREAK*NewZapbreak();
int  GetZapbreakSize();
void*GetZapbreakZpk(ZAPBREAK* pzapbreak);
void SetZapbreakZpk(ZAPBREAK* pzapbreak, ZPK zpk);
void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase);
void AdjustZapbreakNewXp(ZAPBREAK* pzapbreak, XP* pxp, int ixpd);
void UpdateZapbreak(ZAPBREAK* pzapbreak, float dt);
void DeleteZapbreak(ZAPBREAK* pzapbreak);