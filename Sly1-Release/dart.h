#pragma once
#include "so.h"

enum DARTS 
{
    DARTS_Nil = -1,
    DARTS_AvailToFire = 0,
    DARTS_ReadyToFire = 1,
    DARTS_Airborne = 2,
    DARTS_Sticking = 3,
    DARTS_Stuck = 4,
    DARTS_Fading = 5,
    DARTS_Max = 6
};

class DART : public SO
{
	public:
    DARTS darts;
    float tDarts;
    float svDart;
    float sRadiusFireCloud;
    float sRadiusStrikeCloud;
    float dtFade;
    float dtMaxStuck;
    DLE dleDartFree;
    ASEG* pasegSticking;
    ASEGA* pasegaSticking;
    ALO* paloTarget;
    float dtLaunchToTarget;
    float dzTarget;
    struct DARTGUN* pdartgunFiredFrom;
    struct EXPL* pexpl;
};

DART*NewDart();
void InitSwDartFreeDl(SW* psw);
void InitDart(DART* pdart);
void SetDartDarts(DART* pdart, DARTS darts);
void*GetDartDarts(DART* pdart);
void SetDartSvDart(DART* pdart, float svDart);
void*GetDartSvDart(DART* pdart);
void SetDartSRadiusFireCloud(DART* pdart, float sRadiusFireCloud);
void*GetDartSRadiusFireCloud(DART* pdart);
void SetDartSRadiusStrikeCloud(DART* pdart, float sRadiusStrikeCloud);
void*GetDartSRadiusStrikeCloud(DART* pdart);
void SetDartDtFade(DART* pdart, float dtFade);
void*GetDartDtFade(DART* pdart);
void SetDartDtMaxStuck(DART* pdart, float dtMaxStuck);
void*GetDartDtMaxStuck(DART* pdart);
int  GetDartSize();
void OnDartAdd(DART* pdart);
void RemoveDart(DART* pdart);
void CloneDart(DART* pdart, DART* pdartBase);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);
void HandleDartMessage(DART* pdart, MSGID msgid, void* pv);
void PostDartLoad(DART* pdart);
void UpdateDart(DART* pdart, float dt);
void SetDartDarts(DART* pdart, DARTS darts);
void ApplyDartThrow(DART* pdart, PO* ppo);
void UpdateDartAirborne(DART* pdart, float dt);
void DeleteDart(DART *pdart);

extern SNIP s_asnipDartLoad;