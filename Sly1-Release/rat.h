#pragma once
#include "so.h"
#include "chkpnt.h"

enum RATS 
{
    RATS_Nil = -1,
    RATS_Stopped = 0,
    RATS_Scurry = 1,
    RATS_Flee = 2,
    RATS_Dead = 3,
    RATS_Hidden = 4,
    RATS_Max = 5
};

class RAT : public SO
{
	public:
    RATS rats;
    float tRats;
    float tRatsNext;
    float tRatSqueak;
    int ipaloRender;
    int cpaloRenderScurry;
    class ALO* apaloRenderScurry[16];
    int cpaloRenderFlee;
    class ALO* apaloRenderFlee[16];
    class ALO* paloRenderStopped;
    float dsFrameScurry;
    float dsFrameFlee;
    struct EXPL* pexpl;
    struct TARGET* ptarget;
    DLE dleRat;
    LM lmDtStop;
    float sScurry;
    float svScurry;
    float svFlee;
    float svCur;
    float svLeap;
    float sGroupRadius;
    glm::vec3 dposTarget;
    glm::vec3 posWanderNext;
    glm::vec3 posFrameLast;
    struct RATHOLE* prathole;
    int ccoin;
    int fSuppressDifficultyUpdate;
};

class RATHOLE : public VOL
{
	public:
    DLE dleRathole;
};

RAT* NewRat();
void InitSwRatDl(SW* psw);
void InitRat(RAT* prat);
void SetRatSScurry(RAT* prat, float sScurry);
void*GetRatSScurry(RAT* prat);
void SetRatSvScurry(RAT* prat, float svScurry);
void*GetRatSvScurry(RAT* prat);
void SetRatDsFrameScurry(RAT* prat, float dsFrameScurry);
void*GetRatDsFrameScurry(RAT* prat);
void SetRatSvFlee(RAT* prat, float svFlee);
void*GetRatSvFlee(RAT* prat);
void SetRatSGroupRadius(RAT* prat, float sGroupRadius);
void*GetRatSGroupRadius(RAT* prat);
void SetRatLmDtStopGMin(RAT* prat, float gMin);
void*GetRatLmDtStopGMin(RAT* prat);
void SetRatSvLeap(RAT* prat, float svLeap);
void*GetRatSvLeap(RAT* prat);
void*GetRatRats(RAT* prat);
void SetRatCcoin(RAT* prat, int ccoin);
void*GetRatCcoin(RAT* prat);
void SetRatFSuppressDifficultyUpdate(RAT* prat, int fSuppressDifficultyUpdate);
void*GetRatFSuppressDifficultyUpdate(RAT* prat);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void CloneRat(RAT* prat, RAT* pratBase);
void PostRatLoad(RAT* prat);
int  GetRatSize();
void OnRatAdd(RAT* prat);
void OnRatRemove(RAT* prat);
void PresetRatAccel(RAT* prat, float dt);
void UpdateRat(RAT* prat, float dt);
void AdjustRatNewXp(RAT* prat, XP* pxp, int ixpd);
void AdjustRatXpVelocity(RAT* prat, XP* pxp, int ixpd);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);
int  FAbsorbRatWkr(RAT* prat, WKR* pwkr);
void ChooseRatFleeTarget(RAT* prat);
void HideRat(RAT* prat, int fHide);
void SetRatRats(RAT* prat, RATS rats);
void DeleteRat(RAT *prat);

RATHOLE*NewRathole();
void InitSwRatholeDl(SW* psw);
int  GetRatholeSize();
void OnRatholeRemove(RATHOLE* prathole);
void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase);
void OnRatholeAdd(RATHOLE* prathole);
void DeleteRathole(RATHOLE* prathole);

extern SNIP s_asnipRat;
extern SNIP s_asnipPostRatLoad;
extern SMP s_smpRatSpin;
extern CLQ s_clqRatSvNearHole;