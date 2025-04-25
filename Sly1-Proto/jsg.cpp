#include "jsg.h"

JSG* NewJsg()
{
	return new JSG{};
}

void InitJsg(JSG* pjsg)
{
	InitLo(pjsg);
}

int GetJsgSize()
{
	return sizeof(JSG);
}

void CloneJsg(JSG* pjsg, JSG* pjsgBase)
{
    CloneLo(pjsg, pjsgBase);

    // Clone the members
    pjsg->cjsge = pjsgBase->cjsge;

    // Clone the ajsge array (assuming simple array copy)
    pjsg->ajsge = pjsgBase->ajsge;

    // Clone the pjt pointer
    pjsg->pjt = pjsgBase->pjt;

    // Clone the ijsgeCur and tCur
    pjsg->ijsgeCur = pjsgBase->ijsgeCur;
    pjsg->tCur = pjsgBase->tCur;

    // Clone the pjsgeJoy pointer
    pjsg->pjsgeJoy = pjsgBase->pjsgeJoy;

    // Clone the ploContext pointer
    pjsg->ploContext = pjsgBase->ploContext;

    // Clone the pasegaCur pointer
    pjsg->pasegaCur = pjsgBase->pasegaCur;

    // Clone the ploFocus pointer
    pjsg->ploFocus = pjsgBase->ploFocus;

    // Clone the ptnCur pointer
    pjsg->ptnCur = pjsgBase->ptnCur;

    // Clone the fHideTn flag
    pjsg->fHideTn = pjsgBase->fHideTn;
}

void DeleteJsg(JSG* pjsg)
{
	delete pjsg;
}
