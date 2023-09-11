#pragma once
#include "alo.h"
#include "geom.h"

// Static Object
class SO : public ALO
{
public:
	DLE dleRoot;
	DL dlPhys;
	DLE dlePhys;
	glm::mat3 momintLocal;
	glm::mat3 momintInvLocal;
	glm::vec3 dvGravity;
	float gBuoyancy;
	float gViscosity;
	float m;
	glm::vec3 posWorldPrev;
	GEOM geomLocal;
	GEOM geomWorld;
};

void InitSo(SO* pso); // NOT FINISHED
void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis);