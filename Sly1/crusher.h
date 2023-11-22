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

void* NewCrfodb();
void InitCrfodb(CRFODB *pcrfodb);
void DeleteCrfodb(LO* plo);
void*NewCrfod();
void OnCrfodAdd(CRFOD* pcrfod);
void DeleteCrfod(LO* plo);
void* NewCrfodk();
void DeleteCrfodk(LO* plo);
void* NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
void DeleteCrbrain(LO* plo);