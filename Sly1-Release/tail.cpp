#include "tail.h"

TAIL* NewTail()
{
	return new TAIL{};
}

void InitTail(TAIL* ptail)
{
	InitAlo(ptail);
}

int GetTailSize()
{
	return sizeof(TAIL);
}

void CloneTail(TAIL* ptail, TAIL* ptailBase)
{
    CloneAlo(ptail, ptailBase);

    ptail->ptailFirst = ptailBase->ptailFirst;
    ptail->ctsd = ptailBase->ctsd;
    ptail->atsd = ptailBase->atsd;
    ptail->fReset = ptailBase->fReset;
    ptail->rSpring = ptailBase->rSpring;
    ptail->rDampingWorld = ptailBase->rDampingWorld;
    ptail->rDampingLocal = ptailBase->rDampingLocal;
    ptail->radMax = ptailBase->radMax;
    ptail->cIterate = ptailBase->cIterate;
    ptail->dvGravity = ptailBase->dvGravity;
    ptail->fUnlockRot = ptailBase->fUnlockRot;
    ptail->posTip = ptailBase->posTip;
}

void DeleteTail(TAIL* ptail)
{
	delete ptail;
}
