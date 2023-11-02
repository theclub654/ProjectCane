#include "button.h"

void InitButton(BUTTON* pbutton)
{
	InitSo(pbutton);
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
