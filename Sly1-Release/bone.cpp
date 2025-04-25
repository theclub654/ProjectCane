#include "bone.h"

BONE* NewBone()
{
	return new BONE{};
}

int GetBoneSize()
{
	return sizeof(BONE);
}

void CloneBone(BONE* pbone, BONE* pboneBase)
{
	CloneSo(pbone, pboneBase);
}

void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro)
{
	RenderSoSelf(pbone, pcm, pro);
}

void DeleteBone(BONE* pbone)
{
	delete pbone;
}

LBONE*NewLBone()
{
	return new LBONE{};
}

int GetLBoneSize()
{
	return sizeof(LBONE);
}

void CloneLBone(LBONE* plbone, LBONE* plboneBase)
{
	CloneAlo(plbone, plboneBase);
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
	RenderAloSelf(plbone, pcm, pro);
}

void DeleteLBone(LBONE* plbone)
{
	delete plbone;
}