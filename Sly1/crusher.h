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

void InitCrfodb(CRFODB *pcrfodb);
void OnCrfodAdd(CRFOD* pcrfod);
void InitCrbrain(CRBRAIN* pcrbrain);