#include "path.h"

PATHZONE* NewPathzone()
{
    return new PATHZONE{};
}

void InitSwPathzoneDl(SW* psw)
{
    InitDl(&psw->dlPathzone, offsetof(PATHZONE, dlePathzone));
}

int GetPathzoneSize()
{
    return sizeof(PATHZONE);
}

void OnPathzoneAdd(PATHZONE* ppathzone)
{
    OnLoAdd(ppathzone);
    AppendDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

void OnPathzoneRemove(PATHZONE* ppathzone)
{
    OnLoRemove(ppathzone);
    RemoveDlEntry(&ppathzone->psw->dlPathzone, ppathzone);
}

void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis)
{
    LoadLoFromBrx(ppathzone, pbis);
    ppathzone->cg.ccgv = pbis->U16Read();
    ppathzone->cg.acgv.resize(ppathzone->cg.ccgv);

    ppathzone->cg.ccgeBoundary = pbis->U16Read();
    ppathzone->cg.ccge = ppathzone->cg.ccgeBoundary + pbis->U16Read();
    ppathzone->cg.acge.resize(ppathzone->cg.ccge);

    ppathzone->cg.ccgt = pbis->U16Read();
    ppathzone->cg.acgt.resize(ppathzone->cg.ccgt);

    for (int i = 0; i < ppathzone->cg.ccgv; i++)
    {
        pbis->ReadVector();
        byte unk_4 = pbis->U8Read();

        for (int i = 0; i < unk_4; i++)
            pbis->U16Read();
    }


    for (int i = 0; i < ppathzone->cg.ccge; i++)
    {
        for (int i = 0; i < 2; i++)
            pbis->U16Read();
    }

    for (int i = 0; i < ppathzone->cg.ccgt; i++)
    {
        for (int i = 0; i <= 2; i++)
            pbis->U16Read();
    }

    uint16_t unk_5 = pbis->U16Read();
    for (int i = 0; i < unk_5; i++)
    {
        pbis->ReadVector();
        pbis->F32Read();
        pbis->S16Read();
        pbis->S16Read();
    }
}

void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase)
{
    CloneLo(ppathzone, ppathzoneBase);

    ppathzone->cg = ppathzoneBase->cg;
    ppathzone->dlePathzone = ppathzoneBase->dlePathzone;
}

void DeletePathzone(PATHZONE* ppathzone)
{
    delete ppathzone;
}
