#pragma once
#include "stepguard.h"

class CRFOD : public STEPGUARD
{
	public:
};

class CRFODB :public CRFOD
{
	public:
		int fDetectLatch;
		int cpoint;
};

class CRBRAIN : public ALO
{
	public:
		int cpoint;
		int cpointMax;
		float tContest;
};

class CRFODK : public CRFOD
{
	public:
};

void*NewCrfodb();
void InitCrfodb(CRFODB *pcrfodb);
int  GetCrfodbSize();
void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase);
void DeleteCrfodb(LO* plo);

void*NewCrfod();
void OnCrfodAdd(CRFOD* pcrfod);
void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase);
int  GetCrfodSize();
void DeleteCrfod(LO* plo);

void*NewCrfodk();
int  GetCrfodkSize();
void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase);
void DeleteCrfodk(LO* plo);

void*NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
int  GetCrbrainSize();
void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase);
void DeleteCrbrain(LO* plo);