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

PUFFER*NewPuffer();
void InitPuffer(PUFFER *ppuffer);
int  GetPufferSize();
void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis);
void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase);
void DeletePuffer(PUFFER* ppuffer);

PUFFB*NewPuffb();
int  GetPuffbSize();
void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase);
void DeletePuffb(PUFFB* ppuffb);

PUFFV*NewPuffv();
void InitPuffv(PUFFV* ppuffv);
int  GetPuffvSize();
void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase);
void DeletePuffv(LO* plo);

PUFFC*NewPuffc();
int  GetPuffcSize();
void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase);
void DeletePuffc(PUFFC* ppuffc);

PUFFT*NewPufft();
int  GetPufftSize();
void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase);
void DeletePufft(PUFFT *ppufft);