#pragma once
#include "break.h"

class DARTGUN : public BREAK
{
	public:
        OID oidDart;
        int cDarts;
        struct DART* pdartReadyToFire;
        struct DART* pdartFading;
        float dtFireInterval;
        float dzTarget;
        float tLastFire;
        struct ALO* paloTarget;
        struct RAT* pratTarget;
        glm::vec3 posOrigDart;
        glm::mat3 matOrigDart;
        struct ALO* paloOrigDartParent;
        struct ALO* paloLookAt;
        struct ALO* paloLid;
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
int  GetDartgunSize();
void CloneDartgun(DARTGUN* pdartgun, DARTGUN* pdartgunBase);
void BindDartgun(DARTGUN* pdartgun);
void DeleteDartgun(DARTGUN* pdartgun);