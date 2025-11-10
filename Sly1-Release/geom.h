#pragma once
#include "math.h"
#include "bis.h"
#include <glad/glad.h>
#include <vector>

// Collision related

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
    EDGE *pedge;
    EDGE *pedgeOther;
    short grfsurf;
    short ipos;
};

struct GEOM
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    float sRadius;
    int cpos;
    std::vector <glm::vec3> apos;
    int cindices;
    std::vector <uint16_t> indices;
    int csurf;
    std::vector <SURF> asurf;
    int cedge;
    std::vector <EDGE> aedge;
    int *mpiposiiedgeMac;
    int *aiedge;
    std::vector <glm::vec3> mpisurfposCenter;
    std::vector <float> mpisurfsRadius;
};

void InitGeom(GEOM *pgeom);
void ReadGeom(GEOM *pgeom, CBinaryInputStream *pbis);