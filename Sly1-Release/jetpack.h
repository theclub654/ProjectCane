#pragma once
#include "po.h"
#include "asegbl.h"

class RWM;

enum JPK
{
    JETPACKS_Nil = -1,
    JETPACKS_Unknown0 = 0,
    JETPACKS_Unknown1 = 1,
    JETPACKS_Active = 2,
    JETPACKS_Charm = 3,
    JETPACKS_Dead = 4,
    JETPACKS_Max = 5
};

class JETPACK : public PO
{
public:
    JPK jpk;
    float tJetpacks;
    ASEGA* pasegaCur;
    float radBody;
    glm::mat3 matBody;

    RWM* prwm;
    SM* psm;
    SMA* psma;
    ALO* paloTargetOrigin;
    ALO* paloAimBasis;
    ALO* paloAimLimit;
    ALO* paloMovementBasis;
    ALO* paloBodyLeft;
    ALO* paloBodyRight;
    ALO* apaloThruster[2];
    ALO* paloBody;
    ALO* paloEffectBasis;

    BL abl[5];
    ASEGBL* pasegblDynamic;
    ASEG* pasegUnknown1;
    ASEG* pasegCharm;
    ASEG* pasegDead;

    int fFlash;
    float tFlash;
    float tCharm;
    int fCharmEnabled;
    float tCharmPending;
    int fCharmHeld;
    ALO* paloJt;
    ALO* paloEffect;

    float uAimX;
    float uAimY;
    float duAimX;
    float duAimY;
    float radBodyX;
    float radBodyY;
    float dradBodyX;
    float dradBodyY;
    int fExitPending;
};

JETPACK* NewJetpack();
void InitJetpack(JETPACK* pjetpack);
int GetJetpackSize();
void CloneJetpack(JETPACK* pjetpack, JETPACK* pjetpackBase);
void PostJetpackLoad(JETPACK* pjetpack);
void CalculateJetpackTarget(JETPACK* pjetpack, glm::vec3* pposTarget, glm::vec3* pvTarget);
void PresetJetpackAccel(JETPACK* pjetpack, float dt);
void ProjectJetpackTransform(JETPACK* pjetpack, float dt, int fParentDirty);
void UpdateJetpack(JETPACK* pjetpack, float dt);
void UpdateJetpackActive(JETPACK* pjetpack, JOY* pjoy, float dt);
void RenderJetpackAll(JETPACK* pjetpack, CM* pcm, RO* pro);
void OnJetpackActive(JETPACK* pjetpack, int fActive, PO* ppoOther);
int FInvulnerableJetpack(JETPACK* pjetpack, ZPK zpk);
int FTakeJetpackDamage(JETPACK* pjetpack, ZPR* pzpr);
bool JthsCurrentJetpack(JETPACK* pjetpack);
void UseJetpackCharm(JETPACK* pjetpack);
void HandleJetpackMessage(JETPACK* pjetpack, MSGID msgid, void* pv);
void SetJetpackState(JETPACK* pjetpack, JPK jpk);
void ResetJetpackTransform(JETPACK* pjetpack);
void DeleteJetpack(JETPACK* pjetpack);
