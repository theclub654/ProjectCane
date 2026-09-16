#pragma once
#include "lo.h"

struct TRI
{
    int aipos[3];
};

struct MBSP 
{
    glm::vec3 normal;
    float gDot;
    struct MBSP* pmbspNeg;
    struct MBSP* pmbspPos;
};


class MAP : public LO
{
	public:
    int cpos;
    glm::vec3* apos;
    int ctri;
    struct TRI* atri;
    int cmbsp;
    struct MBSP* ambsp;
    int cbEach;
    char* mpiposab;
};

MAP* NewMap();
void CloneMap(MAP* pmap, MAP* pmapBase);
glm::vec3* PvecFromMapIpos(MAP* pmap, int ipos);
void* PvFromMapIpos(MAP* pmap, int ipos);
TRI* PtriFindMap(MAP* pmap, const glm::vec3* ppos);
int  GetMapSize();
void DeleteMap(MAP* pmap);