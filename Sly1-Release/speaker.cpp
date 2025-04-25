#include "speaker.h"

SPEAKER* NewSpeaker()
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
    CloneAlo(pspeaker, pspeakerBase);

    pspeaker->rgbaText = pspeakerBase->rgbaText;

    // Copy basic float values
    pspeaker->radFOV = pspeakerBase->radFOV;
    pspeaker->sDistCm = pspeakerBase->sDistCm;
    pspeaker->uxTv = pspeakerBase->uxTv;
    pspeaker->uyTv = pspeakerBase->uyTv;

    // Copy glm::vec3 values (which are copyable)
    pspeaker->dposLight = pspeakerBase->dposLight;
    pspeaker->dposLightConfront = pspeakerBase->dposLightConfront;

    // Copy confrontation texture UVs
    pspeaker->uxTvConfront = pspeakerBase->uxTvConfront;
    pspeaker->uyTvConfront = pspeakerBase->uyTvConfront;

    // Copy OID (assuming OID is trivially copyable)
    pspeaker->oidSmIdle = pspeakerBase->oidSmIdle;

    // Shallow copy of pointers to SM and SMA structures
    pspeaker->psmIdle = pspeakerBase->psmIdle;
    pspeaker->psmaIdle = pspeakerBase->psmaIdle;
}

void DeleteSpeaker(SPEAKER* pspeaker)
{
	delete pspeaker;
}
