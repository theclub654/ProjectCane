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

BONE*NewBone();
int  GetBoneSize();
void CloneBone(BONE* pbone, BONE* pboneBase);
void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro);
void DeleteBone(BONE* pbone);

LBONE*NewLBone();
int  GetLBoneSize();
void CloneLBone(LBONE* plbone, LBONE* plboneBase);
void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro);
void DeleteLBone(LBONE* plbone);