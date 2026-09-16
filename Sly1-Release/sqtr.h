#pragma once
#include "util.h"
#include "math.h"

struct SQTR 
{
    float t;
    float uAlpha;
    glm::vec3 apos[2];
};

struct SQTRGPU
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec4 rgba;
};

struct SQTRM 
{
    class ALO* palo;
    struct SHD* pshd;
    int isqtrMic;
    int isqtrMac;
    float rAlpha;
    float sRadius;
    int cposLocal;
    glm::vec3 aposLocal[2];
    CLQ clqSvToAlpha;
    CLQ clqDtToAlpha;
    SQTR asqtr[64];
    glm::vec3 posPrev;
    glm::mat3 matPrev;
};

void ClearSqtrm(SQTRM* psqtrm);
void UpdateSqtrm(SQTRM* psqtrm, glm::vec3* ppos, glm::mat3* pmat, float swExtra, float dt);
void RenderSqtrm(SQTRM* psqtrm, CM* pcm);
void DrawSqtrm(RPL* prpl);
