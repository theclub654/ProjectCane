#include "rope.h"

void* NewRope()
{
	return new ROPE;
}

void InitRope(ROPE* prope)
{
	InitAlo(prope);
}

int GetRopeSize()
{
	return sizeof(ROPE);
}

void CloneRope(ROPE* prope, ROPE* propeBase)
{
	LO lo = *prope;
	*prope = *propeBase;
	memcpy(prope, &lo, sizeof(LO));

	CloneLo(prope, propeBase);

	ClearDl(&prope->dlChild);
}

void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro)
{

}

void DeleteRope(LO* plo)
{
	delete(ROPE*)plo;
}
