#pragma once
#include "vec.h"

struct GEOM 
{
    float sRadius;
    int cpos;
    glm::vec3 *apos;
    int csurf;
    struct SURF *asurf;
    int cedge;
    struct EDGE *aedge;
    int *mpiposiiedgeMac;
    int *aiedge;
    glm::vec3 *mpisurfposCenter;
    float *mpisurfsRadius;
};

struct EDGE {
    uint16_t aipos[2];
    uint16_t aisurf[2];
    EDGE *pedgeNext;
    EDGE *pedgeOtherNext;
};

struct SURF {
    glm::vec3 normal;
    float gDot;
    EDGE *pedge;
    EDGE *pedgeOther;
    short grfsurf;
    short ipos;
};

void InitGeom(GEOM *pgeom);