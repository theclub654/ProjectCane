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
	LO lo = *pbone;
	*pbone = *pboneBase;
	memcpy(pbone, &lo, sizeof(LO));

	CloneLo(pbone, pboneBase);

	ClearDl(&pbone->dlChild);

	pbone->pxa = nullptr;
	pbone->grfpvaXpValid = 0;
	pbone->pstso = nullptr;
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
	LO lo = *plbone;
	*plbone = *plboneBase;
	memcpy(plbone, &lo, sizeof(LO));

	CloneLo(plbone, plboneBase);

	ClearDl(&plbone->dlChild);
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
	RenderAloSelf(plbone, pcm, pro);
}

void DeleteLBone(LBONE* plbone)
{
	delete plbone;
}