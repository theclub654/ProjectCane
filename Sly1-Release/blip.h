#pragma once
#include "emitter.h"

struct BLIPGM
{
    EMITDV emitdv;
};

struct BLIPP 
{
    float x;
    float y;
    float z;
    float tCreated;
};

struct BLIPV 
{
    float x;
    float y;
    float z;
    float tDestroy;
};

struct BLIPX 
{
    u32 irgba;
    u32 itex0;
    float swRoll;
    float radRoll;
};

struct BLIPF 
{
    BLIPP ablipp[44];
    BLIPV ablipv[44];
    BLIPX ablipx[44];
};

struct BLIP 
{
    struct BLIPF ablipf[2];
    int iblipfLatest;
    int cblipe;
    struct BLIPSP *pblipsp;
    struct BLIPG *pblipg;
    DLE dle;
};


class BLIPG : public ALO
{
	public:
        struct EMITTER *pemitter;
        CLQ clqScale;
        CLQ clqAlpha;
        CLQ clqTexture;
        CLQ clqColor;
        BLIPMK blipmk;
        BLIPGM blipgm;
        SHD* pshd;
        int crgba;
        glm::vec4 argba[32];
        int fColorRanges;
        BLIPOK blipok;
        float rSFlying;
        int cblipe;
        DL dlBlip;
        DLE dleBlipg;
};

BLIPG*NewBlipg();
void InitSwBlipgDl(SW *psw);
void InitSwBlipgFreeDl(SW *psw);
void InitBlipg(BLIPG *pblipg);
int  GetBlipgSize();
void CloneBlipg(BLIPG *pblipg, BLIPG *pblipgBase);
void OnBlipgAdd(BLIPG *pblipg);
void OnBlipgRemove(BLIPG* pblipg);
void SetBlipgShader(BLIPG *pblipg, OID oid);
void RenderBlipgSelf(BLIPG *pblipg, CM *pcm, RO *pro);
void DeleteBlipg(BLIPG *pblipg);