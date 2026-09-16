#pragma once
#include "po.h"
// GOTTA COME BACK TO THIS

struct LABELMSG
{
    ASEGA* pasega;
    int unknown;
    OID oidLabel;
};
class TURRET : public PO
{
	public:
    class SM* psmTurret;
    class SMA* psmaTurret;
    class RWM* prwm;
    class PNT* ppntAnchor;
    class JT* pjt;
    class MURRAY* pmurray;
    int fCharmEnabled;
    int fSetCameraMatrix;
    int field_0x630;
    int fTrack;
    int cpsoTarget;
    std::vector <SO*> apsoTarget;
    std::vector <int> aoidTarget;
    LOOKK lookk;
    float radPan;
    float radTilt;
    LM lmTilt;
    LM lmPan;
};

TURRET*NewTurret();
int  GetTurretSize();
void InitTurret(TURRET* pturret);
void AddTurretTargetOid(TURRET* pturret, int oidTarget);
void*GetTurretPsmaTurret(TURRET* pturret);
void SetTurretPsmaTurret(TURRET* pturret, SMA* psmaTurret);
void SetTurretfSetCameraMatrix(TURRET* pturret, int fSetCameraMatrix);
void*GetTurretfSetCameraMatrix(TURRET* pturret);
void SetTurretlookk(TURRET* pturret, int lookk);
void*GetTurretlookk(TURRET* pturret);

void SetTurretLmTilt(TURRET* pturret, LM lmTilt);
void*GetTurretLmTilt(TURRET* pturret);
void SetTurretLmPan(TURRET* pturret, LM lmPan);
void*GetTurretLmPan(TURRET* pturret);

void CloneTurret(TURRET* pturret, TURRET* pturretBase);
void PostTurretLoad(TURRET* pturret);
void UpdateTurret(TURRET* pturret, float dt);
void UpdateTurretActive(TURRET* pturret, JOY* pjoy, float dt);
void OnTurretActive(TURRET* pturret, int fActive);
int  FFilterTurret(TURRET* pturret, SO* psoOther);
void UpdateTurretAim(TURRET* pturret);
void FireTurret(TURRET* pturret);
void HandleTurretMessage(TURRET* pturret, MSGID msgid, void* pv);
int  FIgnoreTurretIntersection(TURRET* pturret, SO* psoOther);
void CollectTurretPrize(TURRET* pturret, PCK pck, ALO* paloOther);
void GetTurretDiapi(TURRET* pturret, DIALOG* pdialog, DIAPI* pdiapi);
ALO* PloGetTurretFocus(TURRET* pturret);
void DeleteTurret(TURRET *pturret);

extern SNIP s_asnipTurret[3];
