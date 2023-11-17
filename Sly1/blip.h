#pragma once
#include "emitter.h"

enum BLIPMK 
{
    BLIPMK_Nil = -1,
    BLIPMK_Accel = 0,
    BLIPMK_Constant = 1,
    BLIPMK_Spline = 2,
    BLIPMK_Max = 3
};
enum BLIPOK
{
    BLIPOK_Nil = -1,
    BLIPOK_Fixed = 0,
    BLIPOK_Flying = 1,
    BLIPOK_Rolling = 2,
    BLIPOK_Max = 3
};

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
        DLE dle;
};

void InitBlipg(BLIPG* pblipg);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);