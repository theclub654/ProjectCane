#pragma once
#include "util.h"
#include "vec.h"

struct SQTR 
{
    float t;
    float uAlpha;
    glm::vec3 apos[2];
};

struct SQTRM 
{
    struct ALO* palo;
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

