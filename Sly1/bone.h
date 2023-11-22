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

void* NewBone();
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void DeleteBone(LO* plo);
void* NewLBone();
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LO* plo);