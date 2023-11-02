#include "dialog.h"

void InitDialog(DIALOG* pdialog)
{
	InitAlo(pdialog);
}

void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pdialog, pbis);
	LoadDialogEventsFromBrx(pbis);
	//std::cout << std::hex << pbis->file.tellg();
}

void LoadDialogEventsFromBrx(CBinaryInputStream* pbis)
{
	int8_t unk0 = pbis->S8Read();

	for (int i = 0; i < unk0; i++)
	{
		int8_t unk1 = pbis->S8Read();

		switch (unk1)
		{
		case 10:
		{
			pbis->ReadStringSw();
			break;
		}

		case 0:
		{
			pbis->ReadStringSw();
			break;
		}

		case 1:
		{
			pbis->S16Read();
			pbis->S16Read();
			break;
		}

		case 2:
		{
			pbis->S16Read();
			pbis->S16Read();
			break;
		}

		case 3:
		{
			pbis->F32Read();
			pbis->F32Read();
			pbis->U8Read();
			pbis->U8Read();
			pbis->S16Read();
			pbis->S16Read();
			pbis->S16Read();
			pbis->S16Read();
			pbis->ReadStringSw();
			break;
		}

		case 4:
		{
			pbis->S16Read();
			break;
		}
		}
	}
}
