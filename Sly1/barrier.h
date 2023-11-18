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

void InitBarrier(BARRIER* pbarrier);
void DeleteBarrier(LO* plo);