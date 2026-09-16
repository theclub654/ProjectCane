#pragma once
#include "so.h"

class IKH : public SO
{
	public:
	int fTarget;
	OID oidTarget;
	ALO *ploTarget;
	glm::vec3 vTarget;
};

class LIKH : public ALO
{
	public:
};

IKH* NewIkh();
int  GetIkhSize();
void CloneIkh(IKH* pikh, IKH* pikhBase);
void SetIkhTarget(IKH* pikh, ALO* ploTarget, const glm::vec3* pvTarget);
void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro);
void DeleteIkh(IKH *pikh);

LIKH*NewLikh();
int  GetLikhSize();
void CloneLikh(LIKH* plikh, LIKH* plikhBase);
void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro);
void DeleteLikh(LIKH *plikh);

void SolveAloIK(ALO* palo);