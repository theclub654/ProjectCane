#pragma once
#include "lo.h"

class PNT : public LO
{
	public:
		glm::vec3 posLocal;
};

void* NewPnt();
void LoadPntFromBrx(PNT *pnt, CBinaryInputStream *pbis);
void DeletePnt(LO* plo);