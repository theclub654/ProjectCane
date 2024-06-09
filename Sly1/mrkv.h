#pragma once
#include "so.h"

class MRKV : public SO
{
	public:
		glm::vec3 normal;
		LM lmAlpha;
		float sDepth;
};

MRKV*NewMrkv();
void InitMrkv(MRKV* pmrkv);
int  GetMrkvSize();
void CloneMrkv(MRKV* pmrkv, MRKV* pmrkvBase);
void DeleteMrkv(LO* plo);