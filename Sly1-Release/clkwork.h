#pragma once
#include "so.h"
#include "asegbl.h"

enum CWK 
{ 
    CLKWORKS_Nil = -1, 
    CLKWORKS_Phase0 = 0, 
    CLKWORKS_Phase1 = 1,
    CLKWORKS_Defeated = 2, 
    CLKWORKS_Inactive = 3, 
    CLKWORKS_Max = 4
};

class CLKWORK : public SO
{
    public:
    CWK cwk = CLKWORKS_Nil;
    float tClkworks = 0.0f;
    ASEGA* pasegaState = nullptr;
    BL aablPhase0[4]{};
    ASEGBL* pasegblPhase0 = nullptr;
    BL aablPhase1[4]{};
    ASEGBL* pasegblPhase1 = nullptr;
    BL aablTracking[5]{};
    ASEGBL* pasegblTracking = nullptr;
    ASEGA* pasegaTracking = nullptr;
    SO* psoWeakPoint = nullptr;
    ASEGBL* pasegblDamage = nullptr;
    ASEGA* pasegaDamage = nullptr;
};

CLKWORK* NewClkwork();
void InitClkWork(CLKWORK* pclkwork);
int GetClkworkSize();
void CloneClkwork(CLKWORK* pclkwork, CLKWORK* pclkworkBase);
void DeleteClkwork(CLKWORK* pclkwork);
void PostClkWorkLoad(CLKWORK* pclkwork);
void UpdateClkwork(CLKWORK* pclkwork, float dt);
void UpdateClkworkBlot(CLKWORK* pclkwork);
int FAbsorbClkworkWkr(CLKWORK* pclkwork, WKR* pwkr);
void SetClkworkState(CLKWORK* pclkwork, CWK cwk);
void SetClkworkTracking(CLKWORK* pclkwork, int fTracking);
void SetClkworkDamageAnimation(CLKWORK* pclkwork, OID oid);
void PlayClkworkDamageAnimation(CLKWORK* pclkwork);

enum CWDK : int
{
    CLKWRKDMGS_Inactive = 0,
    CLKWRKDMGS_Active = 1,
    CLKWRKDMGS_Defeated = 2,
    CLKWRKDMGS_Max = 3
};

class CLKWRKDMG : public SO
{
    public:
    CWDK cwdk = CLKWRKDMGS_Inactive;
    float tClkwrkDmg = 0.0f;
    int cHitsRemaining = 0;
    ASEGBL* pasegblDamageDefault = nullptr;
    ASEGBL* pasegblDamageSweepAlternate = nullptr;
    ASEGBL* pasegblDamageSmash = nullptr;
    ASEGA* pasegaDamage = nullptr;
};

CLKWRKDMG* NewClkwrkDmg();
void InitClkwrkDmg(CLKWRKDMG* pclkwrkdmg);
int GetClkwrkDmgSize();
void CloneClkwrkDmg(CLKWRKDMG* pclkwrkdmg, CLKWRKDMG* pclkwrkdmgBase);
void DeleteClkwrkDmg(CLKWRKDMG* pclkwrkdmg);
void PostClkwrkDmgLoad(CLKWRKDMG* pclkwrkdmg);
void UpdateClkwrkDmg(CLKWRKDMG* pclkwrkdmg, float dt);
void UpdateClkwrkDmgBlot(CLKWRKDMG* pclkwrkdmg);
int FAbsorbClkworkDmgWkr(CLKWRKDMG* pclkwrkdmg, WKR* pwkr);
void SetClkwrkDmgState(CLKWRKDMG* pclkwrkdmg, CWDK cwdk);
