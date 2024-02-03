#pragma once
#include "emitter.h"

struct BLIPGM
{
    EMITDV emitdv;
};

class BLIPG : public ALO
{
	public:
        struct EMITTER* pemitter;
        CLQ clqScale;
        CLQ clqAlpha;
        CLQ clqTexture;
        CLQ clqColor;
        BLIPMK blipmk;
        BLIPGM blipgm;
        struct SHD* pshd;
        int cqwTexture;
        int crgba;
        RGBA argba[32];
        int fColorRanges;
        BLIPOK blipok;
        float rSFlying;
        int cblipe;
        DL dlBlip;
        DLE dleBlipg;
};

void*NewBlipg();
void InitSwBlipgDl(SW* psw);
void InitSwBlipgFreeDl(SW* psw);
void InitBlipg(BLIPG* pblipg);
int  GetBlipgSize();
void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);
void DeleteBlipg(LO* plo);