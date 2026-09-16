#pragma once
#include "shdanim.h"

enum EYESS
{
    EYESS_Nil = -1,
    EYESS_Open = 0,
    EYESS_Closing = 1,
    EYESS_Closed = 2,
    EYESS_Opening = 3,
    EYESS_Max = 4
};

struct EYES : public SAA
{
    float dtBlink;
    float dtOpenMin;
    float dtOpenMax;
    float uDoubleBlink;
    OID oidOther;
    SAI saiOther;
    int cframe;
    EYESS eyess;
    float tEyess;
    float dtOpen;
    float sviframe;
    float gframe;
    float uClosed;
};

EYES* NewEyes();
void  LoadEyesFromBrx(EYES* peyes, CBinaryInputStream* pbis);
void  InitEyes(EYES* peyes, SAAF* psaaf);
void  PostEyesLoad(EYES* peyes);
void  UpdateEyes(EYES* peyes, float dt);
void  SetEyesClosed(EYES* peyes, float uClosed);
SAI*  PsaiFromEyesShd(EYES* peyes, SHD* pshd);
void  SetEyesEyess(EYES* peyes, EYESS eyess);
void  DeleteEyes(EYES* peyes);