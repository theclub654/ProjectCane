#pragma once
#include "alo.h"

class SPEAKER : public ALO
{
	public:
        RGBA rgbaText;
        float radFOV;
        float sDistCm;
        float uxTv;
        float uyTv;
        glm::vec3 dposLight;
        glm::vec3 dposLightConfront;
        float uxTvConfront;
        float uyTvConfront;
        OID oidSmIdle;
        struct SM* psmIdle;
        struct SMA* psmaIdle;
};

SPEAKER*NewSpeaker();
void InitSpeaker(SPEAKER* pspeaker);
int  GetSpeakerSize();
void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase);
void DeleteSpeaker(SPEAKER* pspeaker);