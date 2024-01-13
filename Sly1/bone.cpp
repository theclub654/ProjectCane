#include "bone.h"

void* NewBone()
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

void DeleteBone(LO* plo)
{
	delete (BONE*)plo;
}

void* NewLBone()
{
	return new LBONE{};
}

int GetLBoneSize()
{
	return sizeof(LBONE);
}

void CloneLBone(LBONE* lpbone, LBONE* lpboneBase)
{
	LO lo = *lpbone;
	*lpbone = *lpboneBase;
	memcpy(lpbone, &lo, sizeof(LO));

	CloneLo(lpbone, lpboneBase);

	ClearDl(&lpbone->dlChild);
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
	RenderAloSelf(plbone, pcm, pro);
}

void DeleteLBone(LO* plo)
{
	delete (LBONE*)plo;
}
