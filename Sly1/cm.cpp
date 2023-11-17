#include "cm.h"

void InitCm(CM* pcm)
{
	//std::cout << "CM Size: " << sizeof(CM) << "\n";
	InitLo(pcm);

	pcm->radFOV = 45.0;
	pcm->rAspect = 1.298701;
	pcm->sNearClip = 100.0;
	pcm->sFarClip = 20000.0;
	pcm->sNearFog = 500.0;
	pcm->sFarFog = 20000.0;
	pcm->uFogMax = 0.5;
	pcm->rMRD = 1.0;
}
