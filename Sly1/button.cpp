#include "button.h"

void* NewButton()
{
	return new BUTTON;
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
	LO lo = *pbutton;
	*pbutton = *pbuttonBase;
	memcpy(pbutton, &lo, sizeof(LO));

	CloneLo(pbutton, pbuttonBase);

	ClearDl(&pbutton->dlChild);

	pbutton->pxa = nullptr;
	pbutton->grfpvaXpValid = 0;
	pbutton->pstso = nullptr;
}

void DeleteButton(LO* plo)
{
	delete(BUTTON*)plo;
}

void* NewVolbtn()
{
	return new VOLBTN;
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

void DeleteVolbtn(LO* plo)
{
	delete (VOLBTN*)plo;
}
