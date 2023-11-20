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

void*CreateMbg();
void InitMbg(MBG* pmbg);
void*CreateBhg();
void InitBhg(BHG* pbhg);
void DeleteBhg(LO* plo);
void InitScentmap(SCENTMAP* pscentmap);
void DeleteScentmap(LO* plo);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream *pbis);
void DeleteMbg(LO* plo);