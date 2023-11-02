#pragma once
#include "stepguard.h"

class CRFOD : public STEPGUARD
{

};

class CRFODB :public CRFOD
{

};

class CRBRAIN : public ALO
{

};

void InitCrfodb(CRFODB *pcrfodb);
void OnCrfodAdd(CRFOD* pcrfod);
void InitCrbrain(CRBRAIN* pcrbrain);