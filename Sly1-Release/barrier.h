#pragma once
#include "so.h"

enum BWK 
{
    BWK_Nil = -1,
    BWK_None = 0,
    BWK_Pos = 1,
    BWK_Warp = 2,
    BWK_Max = 3
};

struct BARWARP 
{
    BWK bwk;
    union
    {
        glm::vec3 pos;
        short oidWarp;
        struct WARP* pwarp;
    };
};

class BARRIER : public SO
{
	public:
    int fAllow;
    int coid;
    short aoid[4];
    int ccid;
    CID acid[4];
    BARWARP barwarp;
    class SO* psoWarp;
    class SO* psoLastWarped;
};

BARRIER*NewBarrier();
void InitBarrier(BARRIER* pbarrier);
void*GetBarrierFAllow(BARRIER* pbarrier);
void SetBarrierFAllow(BARRIER* pbarrier, int fAllow);
void*GetBarrierOidWarp(BARRIER* pbarrier);
void SetBarrierOidWarp(BARRIER* pbarrier, OID oidWarp);
void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase);
void PostBarrierLoad(BARRIER* pbarrier);
void UpdateBarrier(BARRIER* pbarrier, float dt);
int  FIgnoreBarrierIntersection(BARRIER* pbarrier, SO* psoOther);
void NotifyBarrierImpact(BARRIER* pbarrier, XP* pxp, int ixpd);
void AddBarrierObject(BARRIER* pbarrier, OID oid);
void AddBarrierClass(BARRIER* pbarrier, CID cid);
int  GetBarrierSize();
void DeleteBarrier(BARRIER* pbarrier);