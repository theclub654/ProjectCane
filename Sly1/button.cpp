#include "button.h"

void InitButton(BUTTON* pbutton)
{
	//std::cout << "BUTTON Size: " << sizeof(BUTTON) << "\n";
	InitSo(pbutton);
}

void InitVolbtn(VOLBTN* pvolbtn)
{
	//std::cout << "VOLBTN Size: " << sizeof(VOLBTN) << "\n";
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
