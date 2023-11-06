#pragma once
#include "alo.h"

class BLIPG : public ALO
{
	public:
};

void InitBlipg(BLIPG* pblipg);
void OnBlipgAdd(BLIPG* pblipg);
void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro);