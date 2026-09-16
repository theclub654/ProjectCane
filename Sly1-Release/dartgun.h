#pragma once
#include "break.h"

class DARTGUN : public BREAK
{
	public:
    short oidDart;
    int cDarts;
    struct DART* pdartReadyToFire;
    struct DART* pdartFading;
    float dtFireInterval;
    float dzTarget;
    float tLastFire;
    class ALO* paloTarget;
    struct RAT* pratTarget;
    glm::vec3 posOrigDart;
    glm::mat3 matOrigDart;
    class ALO* paloOrigDartParent;
    class ALO* paloLookAt;
    class ALO* paloLid;
    struct SM* psm;
    struct SMA* psma;
    int coidRatTarget;
    OID aoidRatTarget[8];
    int cpratTarget;
    struct RAT* apratTarget[8];
    int cpratInTargetArea;
    struct RAT* apratInTargetArea[8];
    int fJtInTargetArea;
};

DARTGUN*NewDartgun();
void InitDartgun(DARTGUN* pdartgun);
void SetDartgunOidDart(DARTGUN* pdartgun, OID oidDart);
void*GetDartgunOidDart(DARTGUN* pdartgun);
void SetDartgunCDarts(DARTGUN* pdartgun, int cDarts);
void*GetDartgunCDarts(DARTGUN* pdartgun);
void SetDartgunDtFireInterval(DARTGUN* pdartgun, float dtFireInterval);
void*GetDartgunDtFireInterval(DARTGUN* pdartgun);
void SetDartgunDzTarget(DARTGUN* pdartgun, float dzTarget);
void*GetDartgunDzTarget(DARTGUN* pdartgun);
void HandleDartgunMessage(DARTGUN* pdartgun, MSGID msgid, void* pv);
int  GetDartgunSize();
void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase);
void BindDartgun(DARTGUN* pdartgun);
void SetDartgunGutTarget(DARTGUN* pdartgun);
void PostDartgunLoad(DARTGUN* pdartgun);
void UpdateDartgun(DARTGUN* pdartgun, float dt);
int  FIgnoreDartgunIntersection(DARTGUN* pdartgun, DART* psoOther);
void BreakDartgun(DARTGUN* pdartgun);
void SetDartgunGoalState(DARTGUN* pdartgun, OID oidStateGoal);
void TrackDartgun(DARTGUN* pdartgun, OID* poidStateGoal);
int  FPrepareDartgunToFire(DARTGUN* pdartgun);
void FireDartgun(DARTGUN* pdartgun);
void AgeDartgunStuckDart(DARTGUN* pdartgun);
RAT* PratGetDartgunRatTarget(DARTGUN* pdartgun);
void AddDartgunRatTarget(DARTGUN* pdartgun, OID oid);
void StartDartgunTargetAreaChange(DARTGUN* pdartgun);
void AddDartgunTargetAreaTarget(DARTGUN* pdartgun, ALO* paloTarget);
void DeleteDartgun(DARTGUN* pdartgun);

extern SNIP s_asnipDartgun[3];