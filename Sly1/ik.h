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

void*NewIkh();
int  GetIkhSize();
void CloneIkh(IKH* pikh, IKH* pikhBase);
void DeleteIkh(LO* plo);

void*NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void DeleteLikh(LO* plo);