#pragma once
#include "alo.h"

class DYSH : public ALO
{
	public:
		struct SHADOW* pshadowGen;
};

void*NewDysh();
void InitDysh(DYSH* pdysh);
int  GetDyshSize();
void CloneDysh(DYSH* pdysh, DYSH* pdyshBase);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);
void DeleteDysh(LO* plo);