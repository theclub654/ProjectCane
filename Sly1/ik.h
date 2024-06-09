#pragma once
#include "so.h"

class IKH : public SO
{
	public:
};

class LIKH : public ALO
{
	public:
};

IKH* NewIkh();
int  GetIkhSize();
void CloneIkh(IKH* pikh, IKH* pikhBase);
void DeleteIkh(LO* plo);

LIKH*NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void DeleteLikh(LO* plo);