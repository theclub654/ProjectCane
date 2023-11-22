#include "bone.h"

void* NewBone()
{
	return new BONE;
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
	return new LBONE;
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
	RenderAloSelf(plbone, pcm, pro);
}

void DeleteLBone(LO* plo)
{
	delete (LBONE*)plo;
}
