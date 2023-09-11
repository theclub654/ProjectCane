#pragma once
#include "so.h"

class BUTTON : public SO
{
	public:
};

class VOLBTN : public SO
{
	public:
};

void LoadButtonFromBrx(BUTTON *pbutton, CBinaryInputStream *pbis);
void LoadVolbtnFromBrx(VOLBTN *pvolbtn, CBinaryInputStream *pbis);