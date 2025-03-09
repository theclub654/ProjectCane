#pragma once
#include "stepguard.h"

enum MBAK 
{
    MBAK_Nil = -1,
    MBAK_Gun = 0,
    MBAK_Grenade = 1,
    MBAK_Smash = 2,
    MBAK_Max = 3
};
enum MBSK 
{
    MBSK_Nil = -1,
    MBSK_Intro = 0,
    MBSK_Level1 = 1,
    MBSK_Level2 = 2,
    MBSK_Level3 = 3,
    MBSK_Max = 4
};

class MBG : public STEPGUARD
{
	public:
        MBAK mbak;
        struct ASEG* mpmbakpaseg[3];
        struct RWM* mpmbakprwm[3];
        MBSK mbsk;
        float tMbsk;
        int fAbandonExternal;
        struct XFM* mpmbskpxfm[4];
        BLRUN ablrun[6];
        struct ASEGBL* pasegblRun;
        int fLeftFootDown;
        int fRightFootDown;
};

class BHG : public STEPGUARD
{
    public:
        OID oidScentmap;
        class SCENTMAP* pscentmap;
        float tScent;
};

class SCENTMAP : public ALO
{
	public:
		int cxMax;
		int cyMax;
		int cscp;
		struct SCP* ascp;
		struct SCP** mpixiypscpFirst;
		glm::vec3 posMin;
		glm::vec3 posMax;
};

MBG* NewMbg();
void InitMbg(MBG* pmbg);
int  GetMbgSize();
void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis);
void CloneMbg(MBG* pmbg, MBG* pmbgBase);
void DeleteMbg(MBG* pmbg);

BHG* NewBhg();
void InitBhg(BHG* pbhg);
int  GetBhgSize();
void CloneBhg(BHG* pbhg, BHG* pbhgBase);
void DeleteBhg(BHG* pbhg);

SCENTMAP*NewScentmap();
void InitScentmap(SCENTMAP* pscentmap);
int  GetScentmapSize();
void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase);
void DeleteScentmap(SCENTMAP* pscentmap);