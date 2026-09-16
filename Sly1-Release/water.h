#pragma once
#include "so.h"

struct MSGWATER
{
    WATER* pwater;
    SO* pso;
};

class WATER : public SO
{
	public:
    std::shared_ptr <XA> pxaTargets;
    MRG mrg;
    glm::vec3 vCurrent;
    int fSplash;
    int fZap;
    ZPD zpd;
    glm::vec3 dvCurrent;
    int fSplashDroplets;
    int fWaterWakeSound;
};

WATER*NewWater();
void InitWater(WATER* pwater);
void*GetWaterFSplash(WATER* pwater);
void SetWaterFSplash(WATER* pwater, int fSplash);
void*GetWaterFZap(WATER* pwater);
void SetWaterFZap(WATER* pwater, int fZap);
void*GetWaterVCurrent(WATER* pwater);
void SetWaterVCurrent(WATER* pwater, glm::vec3 vCurrent);
void*GetWaterDzThrow(WATER* pwater);
void SetWaterDzThrow(WATER* pwater, float dzThrow);
void*GetWaterFThrowCut(WATER* pwater);
void SetWaterFThrowCut(WATER* pwater, int fThrowCut);
void*GetWaterFWaterWakeSound(WATER* pwater);
void SetWaterFWaterWakeSound(WATER* pwater, int fWaterWakeSound);
void*GetWaterFSplashDroplets(WATER* pwater);
void SetWaterFSplashDroplets(WATER* pwater, int fSplashDroplets);
void ResetWaterThrowCount(WATER* pwater);
int  GetWaterSize();
void CloneWater(WATER* pwater, WATER* pwaterBase);
void PostWaterLoad(WATER* pwater);
void CalculateWaterCurrent(WATER* pwater, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pw);
void UpdateSwXaList(SW* psw, std::shared_ptr<XA>* ppxa);
void UpdateWater(WATER* pwater, float dt);
void AddWaterExternalAccelerations(WATER* pwater, XA* pxa, float dt);
void HandleWaterMessage(WATER* pwater, MSGID msgid, void* pv);
void UpdateWaterMergeGroup(WATER* pwater);
float UGetWaterSubmerged(WATER* pwater, SO* pso, glm::vec3* pposSurface, glm::vec3* pnormalSurface);
void UpdateWaterBounds(WATER* pwater);
int  FInflictWaterZap(WATER* pwater, XP* pxp, ZPR* pzpr);
void* PzpdEnsureWater(WATER* pwater, ENSK ensk);
void WakeSoWater(SO* pso, WATER* pwater, float dt, glm::vec3* pvCurrent, glm::vec3* pposSurface, float dtWakeMin, float gScale);
void DeleteWater(WATER* pwater);

extern CLQ s_clqDampV;
extern CLQ s_clqDampW;
extern int asfxidWade[4];