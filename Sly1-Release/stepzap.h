#pragma once
#include "step.h"

using PFNZAP = void (*)(void*, PO*);

struct SPKS 
{
    OID oid;
    float sRadius;
    struct ALO* palo;
};

enum SPKK
{
    SPKK_Radial,
    SPKK_Mirror
};

struct SPKR
{
    CLQ clq;
    float v;
    float dz;
    glm::vec4 rgba;
};

struct SPKD
{
    SPKK spkk;
    float uRepeat;
    float svu;
    SPKR aspkr[3];
    SHD* apshd[2];
};

struct ZPR
{
    ZPK zpk;
    struct LO* ploSource;
    void* pv;
    PFNZAP pfnzap;
};

void PostJtLoadZap(JT* pjt);
void PresetJtAccelZap(JT* pjt);
void UpdateJtZap(JT* pjt);
void SetJtPfnZqe(JT* pjt, PFNZQE pfnZqe);
void FinishJtZap(JT* pjt);
bool FTakeJtDamage(JT* pjt, ZPR* pzpr);
void HandleJtDeathOrCharm(JT* pjt);
void StartJtZapSequence(JT* pjt);
void HandleJtZapCharmCheck(JT* pjt);
void RestoreJtDamageVelocity(JT* pjt);
bool FInvulnerableJt(JT* pjt, int damageKind);
void OnJtDead(JT *pjt);
void DrawJtHaloElectric(RPL* prpl);
void DrawJtHaloFire(RPL* prpl);
void DrawHaloSpks(glm::vec3* pposRoot, int cspks, SPKS* aspks, SPKD* pspkd, RPL* prpl);
void RenderJtSelfZap(JT* pjt, CM* pcm, RO* pro);
JTHS JthsCurrentJt(JT* pjt);
void UseJtCharm(JT* pjt);
void UpdateJtWater(JT* pjt);
bool FFindJtWaterSurface(JT* pjt, WATER* pwater, glm::vec3* ppos);
void AddJtWaterAcceleration(JT* pjt, WATER* pwater, float dt);

void InitZpr(ZPR* pzpr, ZPK zpk, LO* ploSource);

inline std::vector<SPKS> s_aspks =
{
    { static_cast<OID>(0x35), 30.0f, nullptr },
    { static_cast<OID>(0x36), 30.0f, nullptr },
    { static_cast<OID>(0x37), 30.0f, nullptr },
    { static_cast<OID>(0x3A), 30.0f, nullptr },
    { static_cast<OID>(0x50), 60.0f, nullptr },
    { static_cast<OID>(0x58), 30.0f, nullptr },
    { static_cast<OID>(0x170), 30.0f, nullptr }
};

static CLQ s_clqZapDamping = { 0.0, 10, 0.0099999998 };
static CLQ s_clqJtZapSoundVolume = { -0.5, -0.0024999999 , 0.0 };
static LM s_lmJtZapSoundVolume = { 0.0, 0.69999999 };

static SMP s_smpJtZapTurn = { 3.5, 0.0, 0.5 };
static glm::vec3 s_vJtDamageZpk4 = { 0.0, 0.0, -1000 };
static float VZ_JtCharmJump = 650;
constexpr float DZ_JtWaterSearchMax = 100.0f;
constexpr float DZ_JtWaterSearchMin = -100.0f;
constexpr float DZ_JtWaterSearchFloat = 50.0f;
constexpr float R_JtWaterDamping = -10.0f;

static SMPA s_smpaJtFloat =
{
    250.0f,  // svFast
    0.0f,    // svSlow
    1.0f,    // dtFast
    4500.0f  // sdvMax
};
