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
void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro);
void DeleteIkh(IKH *pikh);

LIKH*NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro);
void DeleteLikh(LIKH *plikh);