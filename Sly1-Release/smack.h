#pragma once
#include "rip.h"
#include "coin.h"

struct SMACKI
{
    glm::vec3 posTarget;
    glm::vec3 vTarget;
    glm::vec3 vInitMin;
    glm::vec3 vInitMax;
    glm::vec3 normalSpin;
    CLQ clqScale;
    PFNSMACK pfnsmack;
};

struct SMACK : public RIP
{
	glm::vec3 posScreen;
    SMACKI *psmacki;
	void *pv;
};

void InitSmack(SMACK* psmack, glm::vec3* ppos, float scale, SO* psoTouch);
void ProjectSmackTransform(SMACK* psmack, float dt);
void RenderSmack(SMACK* psmack, CM* pcm);
void UpdateSmack(SMACK* psmack, float dt);

struct VTSMACK
{
    void (*pfnInitSmack)(SMACK*, glm::vec3*, float, SO*) = InitSmack;
    void (*pfnPostRipEmit)(RIP*) = nullptr;
    void (*pfnOnRipRemove)(RIP*) = nullptr;
    void (*pfnProjectSmackTransform)(SMACK*, float) = ProjectSmackTransform;
    void (*pfnUpdateSmack)(SMACK*, float) = UpdateSmack;
    void (*pfnRenderSmack)(SMACK*, CM*) = RenderSmack;
    void (*pfnTouchRip)(RIP*, int) = TouchRip;
    int  (*pfnFBounceRip)(RIP*, SO*, glm::vec3*, glm::vec3*) = FBounceRip;
};

extern float DT_SmackLifetime;
extern VTSMACK g_vtsmack;

static SMACKI s_smackiCoin =
{
    glm::vec3(-0.85f,  0.8f, 100.0f), // posTarget
    glm::vec3(-1.0f,  -1.0f,   0.0f), // vTarget
    glm::vec3(-1.0f,   2.0f,   0.0f), // vInitMin
    glm::vec3(1.0f,   3.0f,   0.0f), // vInitMax
    glm::vec3(0.0f,   0.0f,   1.0f), // normalSpin
    CLQ{ 1.0f, -0.8f, 0.0f, 0.0f },  // clqScale
    (PFNSMACK)OnCoinSmack            // pfnsmack
};

static SMACKI g_smackiLifeToken =
{
    glm::vec3(-0.05f,  0.8f, 100.0f), // posTarget
    glm::vec3(-1.0f,  -1.0f,   0.0f), // vTarget
    glm::vec3(-1.0f,   2.0f,   0.0f), // vInitMin
    glm::vec3(1.0f,   3.0f,   0.0f), // vInitMax
    glm::vec3(0.0f,   0.0f,   1.0f), // normalSpin
    CLQ{ 1.0f, -0.8f, 0.0f, 0.0f },  // clqScale
    (PFNSMACK)AwardLife              // 0x00148F80
};