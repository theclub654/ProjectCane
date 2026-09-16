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

struct BLIPSX 
{
    glm::vec3 posFinal;
    glm::vec3 vFinal;
};

struct BLIPSP 
{
    BLIPSX ablipsx[44];
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
    BLIPF ablipf[2];
    int iblipfLatest;
    int cblipe;
    struct BLIPSP *pblipsp;
    struct BLIPG *pblipg;
    DLE dle;
};


class BLIPG : public ALO
{
	public:
    class EMITTER *pemitter;
    CLQ clqScale;
    CLQ clqAlpha;
    CLQ clqTexture;
    CLQ clqColor;
    BLIPMK blipmk;
    // Retail BLIPG + 0x32c.  MGV clears this controller state after writing
    // the two aim values stored in clqScale.g2/gUnused.
    int field_0x32c;
    BLIPGM blipgm;
    SHD* pshd;
    int crgba;
    glm::vec4 argba[32];
    int fColorRanges;
    int cqwTexture;
    int aiframeTexture[32];
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
void PropagateBlipgShader(BLIPG* pblipg);
void PropagateBlipgShaders(GRFZON grfzon);
void SetBlipgEmitb(BLIPG* pblipg, EMITB* pemitb);
void UpdateBlipg(BLIPG* pblipg, float dt);
void SubscribeBlipgObject(BLIPG* pblipg, EMITTER* ploTarget);
void ProjectBlipgTransformAccel(BLIPG* pblipg, float dt);
void ProjectBlipgTransform(BLIPG* pblipg, float dt, int fForce);
void EmitBlips(EMITB* pemitb, EMITG* pemitg, int cblipeRequested, const glm::vec3* apos, const glm::vec3* av, const float* atCreated, const float* atDestroy, const glm::vec3* aposFinal, const glm::vec3* avFinal);
void RenderBlipgSelf(BLIPG *pblipg, CM *pcm, RO *pro);
void DeleteBlipg(BLIPG *pblipg);

BLIP* PblipNew(BLIPG* pblipg);
BLIPG* PblipgNew(SW* psw);
void RemoveBlip(BLIP* pblip);

extern DL g_dlBlipgPending;
extern DL g_dlBlipPending;
extern int g_fDrawEmitterGroupSelf;
