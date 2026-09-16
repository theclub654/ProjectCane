#pragma once

#include "math.h"

class ALO;

enum EMITCNK
{
    EMITCNK_Nil = -1,
    EMITCNK_World = 0,
    EMITCNK_Local = 1,
    EMITCNK_Max = 2
};

struct EMITDV
{
    float rvDamping;
    glm::vec3 dv;
    float swCurl;
    glm::vec3 normalCurl;
    EMITCNK emitcnk;
    ALO* paloCurlRef;
};
