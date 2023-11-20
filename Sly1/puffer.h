#pragma once
#include "stepguard.h"
#include "chkpnt.h"

class PUFFER : public PO
{
	public:
        glm::vec3 normalGround;
        struct VOL* pvolCollect;
        struct WATER* pwater;
        struct VOL* pvolFire;
        struct PNT* appntFloor[4];
        struct ALO* paloFireGun;
        struct ALO* paloFireGunTarget;
        struct ACTLA* pactlaFireGun;
        struct EMITTER* pemitterFire;
        struct EMITTER* pemitterSpray;
        struct PNT* ppntFireNatural;
        struct PUFFT* ppufft;
        struct ALO* paloJt;
        struct ALO* paloGut;
        struct ACTADJ* pactadjGut;
        struct ALO* paloHead;
        struct ACTADJ* pactadjHead;
        int npuffcEaten;
        int npufftLit;
        int npufftMax;
        float tGameMax;
};

class PUFFB : public ALO
{
    public:
        int cppathzone;
        struct PATHZONE** appathzone;
        int* mpippathzonecpuffc;
        int cppuffv;
        struct PUFFV** appuffv;
        int cppuffc;
        struct PUFFC** appuffc;
        float tSpawnNext;
};

class PUFFC : public STEPGUARD
{
	public:
		struct WATER* pwater;
		struct PATHZONE* ppathzoneNext;
		glm::vec3 posPathzoneNext;
		class PUFFV* ppuffvNext;
		struct PUFFB* ppuffb;
};

class PUFFT : public SO
{
    public:
        int fLit;
        struct PNT* ppntFire;
        struct EMITTER* pemitterFire;
};

class PUFFV : public VOL
{
	public:
        OID oidPathzone;
        struct PATHZONE* ppathzone;
        OID oidPuffvJump;
        struct PUFFV* ppuffvJump;
        int coidPuffvLand;
        OID aoidPuffvLand[4];
        int cppuffvLand;
        struct PUFFV* appuffvLand[4];
        float tChosen;
};

void InitPuffer(PUFFER *ppuffer);
void DeletePuffer(LO* plo);
void DeletePuffb(LO* plo);
void InitPuffv(PUFFV* ppuffv);
void DeletePuffv(LO* plo);
void*CreatePuffc();
void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream *pbis);
void DeletePuffc(LO* plo);
void DeletePufft(LO* plo);