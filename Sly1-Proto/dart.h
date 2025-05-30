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
        struct ASEG* pasegSticking;
        struct ASEGA* pasegaSticking;
        struct ALO* paloTarget;
        float dtLaunchToTarget;
        float dzTarget;
        struct DARTGUN* pdartgunFiredFrom;
        struct EXPL* pexpl;
};

DART*NewDart();
void InitSwDartFreeDl(SW* psw);
void InitDart(DART* pdart);
int  GetDartSize();
void OnDartAdd(DART* pdart);
void RemoveDart(DART* pdart);
void CloneDart(DART* pdart, DART* pdartBase);
void SnipDartObjects(DART* pdart);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);
void DeleteDart(DART *pdart);

extern SNIP s_asnip;