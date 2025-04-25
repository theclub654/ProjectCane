#include "dialog.h"

DIALOG* NewDialog()
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
	CloneAlo(pdialog, pdialogBase);

	pdialog->dialogk = pdialogBase->dialogk;
	pdialog->dialogs = pdialogBase->dialogs;
	pdialog->tDialogs = pdialogBase->tDialogs;

	pdialog->cde = pdialogBase->cde;
	pdialog->ade = pdialogBase->ade;

	pdialog->ideCur = pdialogBase->ideCur;
	pdialog->dp = pdialogBase->dp;
	pdialog->oidPntAnchor = pdialogBase->oidPntAnchor;
	pdialog->pfPlayed = pdialogBase->pfPlayed;
	pdialog->fPlayed = pdialogBase->fPlayed;

	pdialog->cpfEquivalence = pdialogBase->cpfEquivalence;
	for (int i = 0; i < 4; ++i)
	{
		pdialog->apfEquivalence[i] = pdialogBase->apfEquivalence[i];
	}

	pdialog->fDefault = pdialogBase->fDefault;
	pdialog->fCrucial = pdialogBase->fCrucial;
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

void DeleteDialog(DIALOG* pdialog)
{
	delete pdialog;
}
