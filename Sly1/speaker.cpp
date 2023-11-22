#include "speaker.h"

void* NewSpeaker()
{
	return new SPEAKER;
}

void InitSpeaker(SPEAKER* pspeaker)
{
	InitAlo(pspeaker);
}

void DeleteSpeaker(LO* plo)
{
	delete(SPEAKER*)plo;
}
