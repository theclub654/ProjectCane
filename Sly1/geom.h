#pragma once
#include "vec.h"

struct EDGE 
{
    uint16_t aipos[2];
    uint16_t aisurf[2];
    EDGE *pedgeNext;
    EDGE *pedgeOtherNext;
};

struct SURF 
{
    glm::vec3 normal;
    float gDot;
    EDGE* pedge;
    EDGE* pedgeOther;
    short grfsurf;
    short ipos;
};

struct GEOM
{
    float sRadius;
    int cpos;
    std::vector <glm::vec3> apos;
    int csurf;
    std::vector <SURF> asurf;
    int cedge;
    std::vector <EDGE> aedge;
    int* mpiposiiedgeMac;
    int* aiedge;
    std::vector <glm::vec3> mpisurfposCenter;
    std::vector <float> mpisurfsRadius;
};

void InitGeom(GEOM *pgeom);