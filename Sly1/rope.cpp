#include "rope.h"

void* NewRope()
{
	return new ROPE;
}

void InitRope(ROPE* prope)
{
	InitAlo(prope);
}

void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro)
{

}

void DeleteRope(LO* plo)
{
	delete(ROPE*)plo;
}
