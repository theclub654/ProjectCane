#pragma once
#include "alo.h"

class DIALOG : public ALO
{
	public:
};

void InitDialog(DIALOG* pdialog);
void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis);
void LoadDialogEventsFromBrx(CBinaryInputStream *pbis);
