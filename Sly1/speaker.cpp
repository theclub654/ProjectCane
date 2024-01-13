#include "speaker.h"

void* NewSpeaker()
{
	return new SPEAKER{};
}

void InitSpeaker(SPEAKER* pspeaker)
{
	InitAlo(pspeaker);
}

int GetSpeakerSize()
{
	return sizeof(SPEAKER);
}

void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase)
{
	LO lo = *pspeaker;
	*pspeaker = *pspeakerBase;
	memcpy(pspeaker, &lo, sizeof(LO));

	CloneLo(pspeaker, pspeakerBase);

	ClearDl(&pspeaker->dlChild);
}

void DeleteSpeaker(LO* plo)
{
	delete(SPEAKER*)plo;
}
