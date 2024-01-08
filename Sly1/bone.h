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

void*NewBone();
int  GetBoneSize();
void CloneBone(BONE* pbone, BONE* pboneBase);
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void DeleteBone(LO* plo);

void*NewLBone();
int  GetLBoneSize();
void CloneLBone(LBONE* lpbone, LBONE* lpboneBase);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LO* plo);