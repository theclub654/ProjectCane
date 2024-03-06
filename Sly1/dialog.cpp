#include "dialog.h"

void* NewDialog()
{
	return new DIALOG{};
}

void InitDialog(DIALOG* pdialog)
{
	InitAlo(pdialog);
}

int GetDialogSize()
{
	return sizeof(DIALOG);
}

void LoadDialogFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pdialog, pbis);
	LoadDialogEventsFromBrx(pdialog, pbis, &pdialog->cde, &pdialog->ade);
}

void CloneDialog(DIALOG* pdialog, DIALOG* pdialogBase)
{
	LO lo = *pdialog;
	*pdialog = *pdialogBase;
	memcpy(pdialog, &lo, sizeof(LO));

	CloneLo(pdialog, pdialogBase);

	ClearDl(&pdialog->dlChild);
}

void LoadDialogEventsFromBrx(DIALOG* pdialog, CBinaryInputStream* pbis, int* pcde, DE** pade)
{
	int8_t numDialogEvents = pbis->S8Read();

	for (int i = 0; i < numDialogEvents; i++)
	{
		DEK dek = (DEK)pbis->S8Read();

		switch (dek)
		{
			case 10:
			case DEK_PreloadVag:
			{
				pbis->ReadStringSw();
				break;
			}

			
			case DEK_SpeakerLeft:
			case DEK_SpeakerRight:
			{
				pbis->S16Read();
				pbis->S16Read();
				break;
			}

			case DEK_Talk:
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

			case DEK_CallSplice:
			{
				pbis->S16Read();
				break;
			}
		}
	}
}

void DeleteDialog(LO* plo)
{
	delete(DIALOG*)plo;
}
