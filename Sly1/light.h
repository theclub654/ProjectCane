#pragma once
#include "alo.h"

extern std::vector<void*> allSwLights;

// Light type
enum LIGHTK 
{
	LIGHTK_Nil = -1,
	LIGHTK_Direction = 0,
	LIGHTK_Position = 1,
	LIGHTK_Frustrum = 2,
	LIGHTK_Spot = 3,
	LIGHTK_Max = 4
};

class LIGHT : public ALO
{
	public:
		LIGHTK lightk;
		TWPS twps;
		glm::vec3 vecHighlight;
		float gMidtone;
		float gShadow;
		float degHighlight;
		float degMidtone;
		float degShadow;
		int fDynamic;
		int fExcludeDynamicObjects;
		glm::vec3 vecDirectionOrig;
		glm::vec3 normalLocal;
		LM lmFallOffS;
		float degCone;
		float degHotSpot;
		glm::vec3 vecUpLocal;
		float rx;
		float ry;
		LM lmFallOffPenumbra;
		LM lmFallOffAbsX;
		LM lmFallOffAbsY;
		glm::vec4 avecFrustrum[6];
		int grfzonBeam;
		glm::mat3 matLookAt;
		DLE dleLight;
};

void*NewLight();
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void AddLightToSw(LIGHT* plight);
void RemoveLightFromSw(LIGHT* plight);
void DeleteLight(LO* plo);