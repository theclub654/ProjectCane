#pragma once
#include "alo.h"

class SPEAKER : public ALO
{
	public:
    RGBA rgbaTextPacked;
    glm::vec4 rgbaText;
    float radFov;
    float sDistCm;
    float uxTv;
    float uyTv;
    glm::vec3 dposLight;
    glm::vec3 dposLightConfront;
    glm::vec3 dposTvConfrontCamera;
    float uxTvConfront;
    float uyTvConfront;
    float radTvConfrontRoll;
    short oidSmIdle;
    struct SM* psmIdle;
    struct SMA* psmaIdle;
};

SPEAKER*NewSpeaker();
void InitSpeaker(SPEAKER* pspeaker);
void*GetSpeakerRgbaText(SPEAKER* pspeaker);
void SetSpeakerRgbaText(SPEAKER* pspeaker, RGBA rgbaText);
void*GetSpeakerRadFov(SPEAKER* pspeaker);
void SetSpeakerRadFov(SPEAKER* pspeaker, float radFov);
void*GetSpeakerSDistCm(SPEAKER* pspeaker);
void SetSpeakerSDistCm(SPEAKER* pspeaker, float sDistCm);
void*GetSpeakerUxTv(SPEAKER* pspeaker);
void SetSpeakerUxTv(SPEAKER* pspeaker, float uxTv);
void*GetSpeakerUyTv(SPEAKER* pspeaker);
void SetSpeakerUyTv(SPEAKER* pspeaker, float uyTv);
void*GetSpeakerDposLight(SPEAKER* pspeaker);
void SetSpeakerDposLight(SPEAKER* pspeaker, glm::vec3 dposLight);
void*GetSpeakerDposLightConfront(SPEAKER* pspeaker);
void SetSpeakerDposLightConfront(SPEAKER* pspeaker, glm::vec3 dposLightConfront);
void*GetSpeakerUxTvConfront(SPEAKER* pspeaker);
void SetSpeakerUxTvConfront(SPEAKER* pspeaker, float uxTvConfront);
void*GetSpeakerUyTvConfront(SPEAKER* pspeaker);
void SetSpeakerUyTvConfront(SPEAKER* pspeaker, float uyTvConfront);
void*GetSpeakerVector310(SPEAKER* pspeaker);
void SetSpeakerVector310(SPEAKER* pspeaker, glm::vec3 value);
void*GetSpeakerFloat328(SPEAKER* pspeaker);
void SetSpeakerFloat328(SPEAKER* pspeaker, float value);
int  GetSpeakerSize();
void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase);
void PostSpeakerLoad(SPEAKER* pspeaker);
void SetSpeakerSmIdle(SPEAKER* pspeaker, OID oidSmIdle);
void*GetSpeakerSmIdle(SPEAKER* pspeaker);
void DeleteSpeaker(SPEAKER* pspeaker);
