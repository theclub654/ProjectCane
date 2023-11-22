#pragma once
#include "so.h"

class MRKV : public SO
{
	public:
		glm::vec3 normal;
		LM lmAlpha;
		float sDepth;
};

void* NewMrkv();
void InitMrkv(MRKV* pmrkv);
void DeleteMrkv(LO* plo);