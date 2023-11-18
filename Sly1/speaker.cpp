#include "speaker.h"

void InitSpeaker(SPEAKER* pspeaker)
{
	//std::cout << "SPEAKER Size: " << sizeof(SPEAKER) << "\n";
	InitAlo(pspeaker);
}

void DeleteSpeaker(LO* plo)
{
	delete(SPEAKER*)plo;
}
