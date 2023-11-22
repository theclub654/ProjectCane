#include "button.h"

void* NewButton()
{
	return new BUTTON;
}

void InitButton(BUTTON* pbutton)
{
	InitSo(pbutton);
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

void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbutton, pbis);
}

void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pvolbtn, pbis);
}

void DeleteVolbtn(LO* plo)
{
	delete (VOLBTN*)plo;
}
