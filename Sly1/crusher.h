#pragma once
#include "stepguard.h"

class CRFOD : public STEPGUARD
{
	public:
		DLE dleCrfod;
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

CRFODB*NewCrfodb();
void InitCrfodb(CRFODB *pcrfodb);
int  GetCrfodbSize();
void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase);
void DeleteCrfodb(CRFODB* pcrfodb);

CRFOD*NewCrfod();
void InitSwCrfodDl(SW* psw);
void OnCrfodAdd(CRFOD* pcrfod);
void OnCrfodRemove(CRFOD* pcrfod);
void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase);
int  GetCrfodSize();
void DeleteCrfodb(CRFODB* pcrfodb);

CRFODK*NewCrfodk();
int  GetCrfodkSize();
void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase);
void DeleteCrfodk(CRFODK* pcrfodk);

CRBRAIN*NewCrbrain();
void InitCrbrain(CRBRAIN* pcrbrain);
int  GetCrbrainSize();
void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase);
void DeleteCrbrain(CRBRAIN* pcrbrain);