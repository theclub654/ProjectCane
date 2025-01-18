#pragma once
#include "step.h"

enum JBS 
{
    JBS_Nil = -1,
    JBS_Chase = 0,
    JBS_Spike = 1,
    JBS_Fly = 2,
    JBS_Taunt = 3,
    JBS_Max = 4
};
enum JBWS
{
    JBWS_Nil = -1,
    JBWS_PlantLeft = 0,
    JBWS_PlantRight = 1,
    JBWS_PlantNone = 2,
    JBWS_Max = 3
};
enum JACKS
{
    JACKS_Nil = -1,
    JACKS_Rim = 0,
    JACKS_Gap = 1,
    JACKS_Zap = 2,
    JACKS_Dead = 3,
    JACKS_Celebrate = 4,
    JACKS_Max = 5
};
struct JE 
{
    int i;
    int j;
};
struct JPR 
{
    JE je;
    struct SO* pso;
};

class JACK : public PO
{
	public:
        JACKS jacks;
        float tJacks;
        struct ASEGA* pasegaCur;
        struct ASEG* pasegStop;
        struct ASEG* pasegRim;
        struct ASEG* pasegGap;
        struct ASEG* pasegZap;
        struct ASEG* pasegDead;
        struct ASEG* pasegCelebrate;
        struct JACKF* pjackf;
        struct JACKB* pjackb;
        struct JACKN* apjackn[32];
        struct JACKF* apjackfDrop[4];
        int cjpr;
        JPR ajpr[6];
        int iCur;
        int jCur;
        int iNext;
        int jNext;
        float uCur;
        float radTarget;
        int cpjacknActive;
        int cpjacknTarget;
        float tNextJackn;
        struct SHD* pshdTop;
        struct SHD* pshdSides;
        struct SHD* pshdGap;
        glm::mat4 matPosToUv;
};
class JACKN : public ALO
{
	public:
        struct JACK* pjack;
        struct JACKF* pjackf;
        int cpaloRender;
        struct ALO* apaloRender[16];
        float svu;
        float u;
        int fGap;
};

class JACKB : public ALO
{
	public:
        struct JACK* pjack;
        JBS jbs;
        float tJbs;
        struct ASEGA* pasegaCur;
        JBS jbsNext;
        float radTarget;
        BL ablWalk[2];
        struct ASEGBL* pasegblWalk;
        struct ASEG* pasegSpike;
        struct ASEG* pasegFly;
        struct ASEG* pasegTaunt;
        struct PNT* ppntLeftWingTip;
        struct PNT* ppntRightWingTip;
        JBWS jbws;
        JBWS jbwsNext;
        glm::vec3 posPlant;
        glm::vec3 posNextPlant;
};

class JACKF : public ALO
{
	public:
        struct JACK* pjack;
        int cjeRim;
        struct JE* ajeRim;
        struct JEP* ajepRim;
        int cjeGap;
        struct JE* ajeGap;
        enum JK* aaajk;
        float tDrop;
        glm::vec3 dvDrop;
        int c;
};

JACK*NewJack();
void InitJack(JACK *pjack);
int  GetJackSize();
void CloneJack(JACK* pjack, JACK* pjackBase);
void DeleteJack(JACK* pjack);

JACKB*NewJackb();
void InitJackb(JACKB* pjackb);
int  GetJackbSize();
void CloneJackb(JACKB* pjackb, JACKB* pjackbBase);
void DeleteJackb(LO* plo);

JACKN*NewJackn();
void InitJackn(JACKN* pjackn);
int  GetJacknSize();
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);
void CloneJackn(JACKN* pjackn, JACKN* pjacknBase);
void UpdateJackn(JACKN* pjackn, float dt);
void RenderJacknSelf(JACKN* pjackn, CM* pcm, RO* pro);
void DeleteJackn(LO* plo);

JACKF*NewJackf();
void InitJackf(JACKF* pjackf);
int  GetJackfSize();
void CloneJackf(JACKF* pjackf, JACKF* pjackfBase);
void RenderJackfSelf(JACKF* pjackf, CM* pcm, RO* pro);
void DeleteJackf(LO* plo);