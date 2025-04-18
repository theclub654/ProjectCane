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
        struct ALO* apaloRenderScurry[16];
        int cpaloRenderFlee;
        struct ALO* apaloRenderFlee[16];
        struct ALO* paloRenderStopped;
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
};

class RATHOLE : public VOL
{
	public:
        DLE dleRathole;
};

RAT* NewRat();
void InitSwRatDl(SW* psw);
void InitRat(RAT* prat);
int  GetRatSize();
void OnRatAdd(RAT* prat);
void OnRatRemove(RAT* prat);
void CloneRat(RAT* prat, RAT* pratBase);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);
void RenderRatAll(RAT* prat, CM* pcm, RO* pro);
void DeleteRat(RAT *prat);

RATHOLE*NewRathole();
void InitSwRatholeDl(SW* psw);
int  GetRatholeSize();
void OnRatholeRemove(RATHOLE* prathole);
void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase);
void OnRatholeAdd(RATHOLE* prathole);
void DeleteRathole(RATHOLE* prathole);