#include "rope.h"

ROPE* NewRope()
{
	return new ROPE{};
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
    CloneAlo(prope, propeBase); // Assuming CloneAlo handles copying the base class (ALO) members

    // Shallow copy of value members
    prope->ropek = propeBase->ropek;
    prope->oidOther = propeBase->oidOther;
    prope->oidShd = propeBase->oidShd;
    prope->sLength = propeBase->sLength;
    prope->dsSlack = propeBase->dsSlack;
    prope->r = propeBase->r;
    prope->cLength = propeBase->cLength;
    prope->cAxis = propeBase->cAxis;
    prope->sRadius = propeBase->sRadius;
    prope->vMax = propeBase->vMax;

    // Shallow copy of pointer members
    prope->ppntOther = propeBase->ppntOther;
    prope->pshd = propeBase->pshd;

    // Shallow copy of the CLQ (assuming it's a copyable object)
    prope->clqSToUMax = propeBase->clqSToUMax;
}

void RenderRopeAll(ROPE* prope, CM* pcm, RO* pro)
{
	RenderRopeAll(prope, pcm, pro);
}

void DeleteRope(ROPE* prope)
{
	delete prope;
}
