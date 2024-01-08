#pragma once
#include "so.h"

struct BARWARP 
{
    enum BWK bwk;
    union
    {
        glm::vec3 pos;
        OID oidWarp;
        struct WARP* pwarp;
    };
};

class BARRIER : public SO
{
	public:
        int fAllow;
        int coid;
        OID aoid[4];
        int ccid;
        CID acid[4];
        BARWARP barwarp;
        struct SO* psoWarp;
        struct SO* psoLastWarped;
};

void*NewBarrier();
void InitBarrier(BARRIER* pbarrier);
int  GetBarrierSize();
void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase);
void DeleteBarrier(LO* plo);