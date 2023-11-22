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

void* NewIkh();
void DeleteIkh(LO* plo);
void* NewLikh();
void DeleteLikh(LO* plo);