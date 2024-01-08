#pragma once
#include "po.h"

class TURRET : public PO
{
	public:
        struct SM* psm;
        struct SMA* psma;
        struct RWM* prwm;
        struct PNT* ppntAnchor;
        struct MURRAY* pmurray;
        int fCharmEnabled;
        float tCharmPending;
};

void*NewTurret();
int  GetTurretSize();
void CloneTurret(TURRET* pturret, TURRET* pturretBase);
void DeleteTurret(LO* plo);