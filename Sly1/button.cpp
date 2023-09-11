#include "button.h"

void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbutton, pbis);
}

void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pvolbtn, pbis);
}
