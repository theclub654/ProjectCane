#include "button.h"

BUTTON* NewButton()
{
	return new BUTTON{};
}

void InitButton(BUTTON* pbutton)
{
	InitSo(pbutton);
}

int GetButtonSize()
{
	return sizeof(BUTTON);
}

void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbutton, pbis);
}

void CloneButton(BUTTON* pbutton, BUTTON* pbuttonBase)
{
    CloneSo(pbutton, pbuttonBase);

    pbutton->btnt = pbuttonBase->btnt;
    pbutton->btn = pbuttonBase->btn;
    pbutton->coidPush = pbuttonBase->coidPush;
    for (int i = 0; i < 8; ++i)
        pbutton->aoidPush[i] = pbuttonBase->aoidPush[i];

    pbutton->coidNoPush = pbuttonBase->coidNoPush;
    for (int i = 0; i < 8; ++i)
        pbutton->aoidNoPush[i] = pbuttonBase->aoidNoPush[i];

    pbutton->ccidPush = pbuttonBase->ccidPush;
    for (int i = 0; i < 8; ++i)
        pbutton->acidPush[i] = pbuttonBase->acidPush[i];

    pbutton->ccidNoPush = pbuttonBase->ccidNoPush;
    for (int i = 0; i < 8; ++i)
        pbutton->acidNoPush[i] = pbuttonBase->acidNoPush[i];

    pbutton->istopClosestCur = pbuttonBase->istopClosestCur;
    pbutton->cposStop = pbuttonBase->cposStop;
    for (int i = 0; i < 2; ++i)
        pbutton->aposStop[i] = pbuttonBase->aposStop[i];

    pbutton->cmatStop = pbuttonBase->cmatStop;
    for (int i = 0; i < 2; ++i)
        pbutton->amatStop[i] = pbuttonBase->amatStop[i];

    pbutton->posLever = pbuttonBase->posLever;
    for (int i = 0; i < 2; ++i)
        pbutton->anormLimits[i] = pbuttonBase->anormLimits[i];

    pbutton->phndSwitchHook = pbuttonBase->phndSwitchHook;
}

void DeleteButton(BUTTON *pbutton)
{
	delete pbutton;
}

VOLBTN* NewVolbtn()
{
	return new VOLBTN{};
}

void InitVolbtn(VOLBTN* pvolbtn)
{
	InitSo(pvolbtn);
}

int GetVolbtnSize()
{
	return sizeof(VOLBTN);
}

void CloneVolbtn(VOLBTN* pvolbtn, VOLBTN* pvolbtnBase)
{
	LO lo = *pvolbtn;
	*pvolbtn = *pvolbtnBase;
	memcpy(pvolbtn, &lo, sizeof(LO));

	CloneLo(pvolbtn, pvolbtnBase);

	ClearDl(&pvolbtn->dlChild);

	pvolbtn->pxa = nullptr;
	pvolbtn->grfpvaXpValid = 0;
	pvolbtn->pstso = nullptr;
}

void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pvolbtn, pbis);
}

void DeleteVolbtn(VOLBTN* pvolbtn)
{
	delete pvolbtn;
}
