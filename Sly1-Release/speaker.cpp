#include "speaker.h"
#include "sm.h"

namespace
{
glm::vec4 NormalizeSpeakerTextColor(const RGBA& rgba)
{
    constexpr float byteToFloat = 1.0f / 255.0f;
    return glm::vec4(
        static_cast<float>(rgba.bRed) * byteToFloat,
        static_cast<float>(rgba.bGreen) * byteToFloat,
        static_cast<float>(rgba.bBlue) * byteToFloat,
        static_cast<float>(rgba.bAlpha) * byteToFloat);
}
}

SPEAKER* NewSpeaker()
{
	return new SPEAKER{};
}

void InitSpeaker(SPEAKER* pspeaker)
{
    InitAlo(pspeaker);

    pspeaker->oidSmIdle = OID_Nil;

    pspeaker->rgbaTextPacked = { 255, 255, 255, 255 };
    pspeaker->rgbaText = NormalizeSpeakerTextColor(pspeaker->rgbaTextPacked);

    pspeaker->sDistCm = 400.0f;

    pspeaker->uxTvConfront = 0.3f;
    pspeaker->uyTvConfront = 0.5f;

    pspeaker->dposLight.x = 50.0f;
    pspeaker->dposLight.y = 0.0f;
    pspeaker->dposLight.z = 0.0f;

    pspeaker->dposLightConfront.x = 50.0f;
    pspeaker->dposLightConfront.y = 100.0f;
    pspeaker->dposLightConfront.z = 80.0f;

    pspeaker->radFov = 0.5f;

    pspeaker->uxTv = 0.5f;
    pspeaker->uyTv = 0.5f;
}

void* GetSpeakerRgbaText(SPEAKER* pspeaker)
{
    return &pspeaker->rgbaTextPacked;
}

void SetSpeakerRgbaText(SPEAKER* pspeaker, RGBA rgbaText)
{
    pspeaker->rgbaTextPacked = rgbaText;
    pspeaker->rgbaText = NormalizeSpeakerTextColor(pspeaker->rgbaTextPacked);
}

void* GetSpeakerRadFov(SPEAKER* pspeaker)
{
    return &pspeaker->radFov;
}

void SetSpeakerRadFov(SPEAKER* pspeaker, float radFov)
{
    pspeaker->radFov = radFov;
}

void* GetSpeakerSDistCm(SPEAKER* pspeaker)
{
    return &pspeaker->sDistCm;
}

void SetSpeakerSDistCm(SPEAKER* pspeaker, float sDistCm)
{
    pspeaker->sDistCm = sDistCm;
}

void* GetSpeakerUxTv(SPEAKER* pspeaker)
{
    return &pspeaker->uxTv;
}

void SetSpeakerUxTv(SPEAKER* pspeaker, float uxTv)
{
    pspeaker->uxTv = uxTv;
}

void* GetSpeakerUyTv(SPEAKER* pspeaker)
{
    return &pspeaker->uyTv;
}

void SetSpeakerUyTv(SPEAKER* pspeaker, float uyTv)
{
    pspeaker->uyTv = uyTv;
}

void* GetSpeakerDposLight(SPEAKER* pspeaker)
{
    return &pspeaker->dposLight;
}

void SetSpeakerDposLight(SPEAKER* pspeaker, glm::vec3 dposLight)
{
    pspeaker->dposLight = dposLight;
}

void* GetSpeakerDposLightConfront(SPEAKER* pspeaker)
{
    return &pspeaker->dposLightConfront;
}

void SetSpeakerDposLightConfront(SPEAKER* pspeaker, glm::vec3 dposLightConfront)
{
    pspeaker->dposLightConfront = dposLightConfront;
}

void* GetSpeakerUxTvConfront(SPEAKER* pspeaker)
{
    return &pspeaker->uxTvConfront;
}

void SetSpeakerUxTvConfront(SPEAKER* pspeaker, float uxTvConfront)
{
    pspeaker->uxTvConfront = uxTvConfront;
}

void* GetSpeakerUyTvConfront(SPEAKER* pspeaker)
{
    return &pspeaker->uyTvConfront;
}

void SetSpeakerUyTvConfront(SPEAKER* pspeaker, float uyTvConfront)
{
    pspeaker->uyTvConfront = uyTvConfront;
}

void* GetSpeakerVector310(SPEAKER* pspeaker)
{
    return &pspeaker->dposTvConfrontCamera;
}

void SetSpeakerVector310(SPEAKER* pspeaker, glm::vec3 value)
{
    pspeaker->dposTvConfrontCamera = value;
}

void* GetSpeakerFloat328(SPEAKER* pspeaker)
{
    return &pspeaker->radTvConfrontRoll;
}

void SetSpeakerFloat328(SPEAKER* pspeaker, float value)
{
    pspeaker->radTvConfrontRoll = value;
}

int GetSpeakerSize()
{
	return sizeof(SPEAKER);
}

void CloneSpeaker(SPEAKER* pspeaker, SPEAKER* pspeakerBase)
{
    CloneAlo(pspeaker, pspeakerBase);

    pspeaker->rgbaTextPacked = pspeakerBase->rgbaTextPacked;
    pspeaker->rgbaText = pspeakerBase->rgbaText;

    // Copy basic float values
    pspeaker->radFov = pspeakerBase->radFov;
    pspeaker->sDistCm = pspeakerBase->sDistCm;
    pspeaker->uxTv = pspeakerBase->uxTv;
    pspeaker->uyTv = pspeakerBase->uyTv;

    // Copy glm::vec3 values (which are copyable)
    pspeaker->dposLight = pspeakerBase->dposLight;
    pspeaker->dposLightConfront = pspeakerBase->dposLightConfront;
    pspeaker->dposTvConfrontCamera = pspeakerBase->dposTvConfrontCamera;

    // Copy confrontation texture UVs
    pspeaker->uxTvConfront = pspeakerBase->uxTvConfront;
    pspeaker->uyTvConfront = pspeakerBase->uyTvConfront;
    pspeaker->radTvConfrontRoll = pspeakerBase->radTvConfrontRoll;

    // Copy OID (assuming OID is trivially copyable)
    pspeaker->oidSmIdle = pspeakerBase->oidSmIdle;

    // Shallow copy of pointers to SM and SMA structures
    pspeaker->psmIdle = pspeakerBase->psmIdle;
    pspeaker->psmaIdle = pspeakerBase->psmaIdle;
}

void PostSpeakerLoad(SPEAKER* pspeaker)
{
    PostAloLoad(pspeaker);
    pspeaker->rgbaText = NormalizeSpeakerTextColor(pspeaker->rgbaTextPacked);
    SetSpeakerSmIdle(pspeaker, (OID)pspeaker->oidSmIdle);
    pspeaker->pvtlo->pfnRemoveLo(pspeaker);
}

void SetSpeakerSmIdle(SPEAKER* pspeaker, OID oidSmIdle)
{
    if (pspeaker->psmaIdle)
    {
        RetractSma(pspeaker->psmaIdle);
        pspeaker->psmaIdle = nullptr;
    }

    pspeaker->psmIdle = (SM*)PloFindSwObject(pspeaker->psw, 0x101, oidSmIdle, pspeaker);

    if (pspeaker->psmIdle)
        pspeaker->psmaIdle = PsmaApplySm(pspeaker->psmIdle, (ALO*)pspeaker, OID_Nil, 1);

    pspeaker->oidSmIdle = oidSmIdle;
}

void* GetSpeakerSmIdle(SPEAKER* pspeaker)
{
    return &pspeaker->oidSmIdle;
}

void DeleteSpeaker(SPEAKER* pspeaker)
{
	delete pspeaker;
}
