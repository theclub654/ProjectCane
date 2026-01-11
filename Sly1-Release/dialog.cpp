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
    int8_t unk_0 = pbis->S8Read();

    for (int i = 0; i < unk_0; i++)
    {
        int8_t unk_1 = pbis->S8Read();

        switch (unk_1)
        {
        case 0:
            pbis->file.seekg(0x20, SEEK_CUR);
            break;
        case 7:
            pbis->file.seekg(0x20, SEEK_CUR);
            break;

        case 1:
            pbis->S16Read();
            pbis->S16Read();
            break;

        case 2:
            pbis->S16Read();
            pbis->S16Read();
            break;

        case 3:
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

        case 4:
            pbis->S16Read();
            break;

        }
    }
}

void UpdateDialog(DIALOG* pdialog, float dt)
{
    UpdateAlo(pdialog, dt);
}

void DeleteDialog(DIALOG* pdialog)
{
	delete pdialog;
}
