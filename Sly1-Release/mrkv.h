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
void*GetMrkvNormal(MRKV* pmrkv);
void SetMrkvNormal(MRKV* pmrkv, glm::vec3 normal);
void*GetMrkvSDepth(MRKV* pmrkv);
void SetMrkvSDepth(MRKV* pmrkv, float sDepth);
void*GetMrkvLmAlpha(MRKV* pmrkv);
void SetMrkvLmAlpha(MRKV* pmrkv, LM lmAlpha);
int  GetMrkvSize();
void CloneMrkv(MRKV* pmrkv, MRKV* pmrkvBase);
void DeleteMrkv(MRKV *pmrkv);