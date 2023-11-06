#pragma once
#include "so.h"

class BONE : public SO
{
	public:
};

class LBONE : public ALO
{
	public:
};

void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);