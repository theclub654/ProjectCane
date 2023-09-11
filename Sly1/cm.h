#pragma once
#include "lo.h"

// Camera Object
class CM : public LO
{
public:
	glm::vec3 pos;
	glm::vec4 anormalFrustrumTranspose;
	glm::mat3 mat;
	float rMRDAdjust;
	glm::mat4 matProj;
	glm::mat4 matWorldToClip;
	glm::mat4 matClipToWorld;
	glm::vec4 anormalFrustrum;
	float rMRD;
	float radFOV;
	float rAspect;
	float sNearClip;
	float sFarClip;
	float sRadiusNearClip;
	float xScreenRange;
	float yScreenRange;
	float sNearFog;
	float sFarFog;
	float uFogMax;

	CM(CID cid, SW* psw, ALO* paloParent, OID oid);
};

// Initialize camera object
void InitCm(CM* pcm); // NOT FINISHED

// Global pointer to parent camera object
static CM* g_pcm;