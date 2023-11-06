#include "bone.h"

void RenderBoneSelf(BONE* pbone, CM* pcm, RO* pro)
{
	RenderSoSelf(pbone, pcm, pro);
}

void RenderLboneSelf(LBONE* plbone, CM* pcm, RO* pro)
{
	RenderAloSelf(plbone, pcm, pro);
}
