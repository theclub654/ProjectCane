#pragma once
#include "explo.h"

class EXPLS : public EXPLO
{
    public:
    std::shared_ptr <SFX> psfx;
    LM lmcParticle;
    short oidRender;
    short oidNextRender;
    short oidTouch;
    float dtDelay;
    int fGrouped;
    struct RIPG* pripg;
    struct BLIPG* pblipg;
    float tExplodeNext;
    int fExplodeSiblings;
};

EXPLS* NewExpls();
void InitExpls(EXPLS* pexpls);
void* GetExplsLmcParticle(EXPLS* pexpls);
void SetExplsLmcParticle(EXPLS* pexpls, LM lmcParticle);
void* GetExplsOidRender(EXPLS* pexpls);
void SetExplsOidRender(EXPLS* pexpls, int oidRender);
void* GetExplsOidNextRender(EXPLS* pexpls);
void SetExplsOidNextRender(EXPLS* pexpls, int oidNextRender);
void* GetExplsOidTouch(EXPLS* pexpls);
void SetExplsOidTouch(EXPLS* pexpls, int oidTouch);
void* GetExplsDtDelay(EXPLS* pexpls);
void SetExplsDtDelay(EXPLS* pexpls, float dtDelay);
void* GetExplsFGrouped(EXPLS* pexpls);
void SetExplsFGrouped(EXPLS* pexpls, int fGrouped);
glm::vec3* PemitbEnsureExplsDv(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsRvDamping(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsSwCurl(EXPLS* pexpls, ENSK ensk);
glm::vec3* PemitbEnsureExplsNormalCurl(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsEmitcnk(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsDtLifetime(EXPLS* pexpls, ENSK ensk);
LM* PemitbEnsureExplsLmDtBirth(EXPLS* pexpls, ENSK ensk);
CLQ* PemitbEnsureExplsClqAlpha(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsEmitpk(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsRiptTrail(EXPLS* pexpls, ENSK ensk);
LM* PemitbEnsureExplsLmGScale(EXPLS* pexpls, ENSK ensk);
CLQ* PemitbEnsureExplsRipClqScale(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsRipgt(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsSExpand(EXPLS* pexpls, ENSK ensk);
OID* PemitbEnsureExplsOidShader(EXPLS* pexpls, ENSK ensk);
CLQ* PemitbEnsureExplsBlipClqScale(EXPLS* pexpls, ENSK ensk);
CLQ* PemitbEnsureExplsClqTexture(EXPLS* pexpls, ENSK ensk);
CLQ* PemitbEnsureExplsClqColor(EXPLS* pexpls, ENSK ensk);
int* PemitbEnsureExplsFShaderSpan(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsDtShaderLoop(EXPLS* pexpls, ENSK ensk);
int* PemitbEnsureExplsFRandomFrame(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsBlipmk(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsBlipok(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsRSFlying(EXPLS* pexpls, ENSK ensk);
void* PemitbEnsureExplsBliprk(EXPLS* pexpls, ENSK ensk);
LM* PemitbEnsureExplsLmSw(EXPLS* pexpls, ENSK ensk);
int* PemitbEnsureExplsFRandomRoll(EXPLS* pexpls, ENSK ensk);
int* PemitbEnsureExplsBulletFDamage(EXPLS* pexpls, ENSK ensk);
int* PemitbEnsureExplsBulletFRichochet(EXPLS* pexpls, ENSK ensk);
OID* PemitbEnsureExplsRemitOidExpls(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsRemitSvcParticle(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsShrapnelSRadius(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsShrapnelElas(EXPLS* pexpls, ENSK ensk);
float* PemitbEnsureExplsShrapnelMu(EXPLS* pexpls, ENSK ensk);
SFXID* PsfxEnsureExplsSfxid(EXPLS* pexpls, ENSK ensk);
float* PsfxEnsureExplsSStart(EXPLS* pexpls, ENSK ensk);
float* PsfxEnsureExplsSFull(EXPLS* pexpls, ENSK ensk);
float* PsfxEnsureExplsUVol(EXPLS* pexpls, ENSK ensk);
int  GetExplsSize();
void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase);
void BindExpls(EXPLS* pexpls);
void HandleExplsMessage(EXPLS* pexpls, MSGID msgid, void* pv);
void ExplodeExplsExplso(EXPLS* pexpls, EXPLSO* pexplso);
SFX* PsfxEnsureExpls(EXPLS* pexpls, ENSK ensk);
void DeleteExpls(EXPLS* pexpls);