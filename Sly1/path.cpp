#include "path.h"

void* NewPathzone()
{
    return new PATHZONE{};
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

void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis)
{
    LoadLoFromBrx(ppathzone, pbis);
    uint16_t unk_0 = pbis->U16Read();
    uint16_t unk_1 = pbis->U16Read();
    uint16_t unk_2 = pbis->U16Read();
    uint16_t unk_3 = pbis->U16Read();

    uint16_t unk = unk_1 + unk_2;

    for (int i = 0; i < unk_0; i++)
    {
        pbis->ReadVector();
        byte unk_4 = pbis->U8Read();

        for (int i = 0; i < unk_4; i++)
            pbis->U16Read();
    }


    for (int i = 0; i < unk; i++)
    {
        for (int i = 0; i < 2; i++)
            pbis->U16Read();
    }

    for (int i = 0; i < unk_3; i++)
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
    LO lo = *ppathzone;
    *ppathzone = *ppathzoneBase;
    memcpy(ppathzone, &lo, sizeof(LO));

    CloneLo(ppathzone, ppathzoneBase);
}

void DeletePathzone(LO* plo)
{
    delete(PATHZONE*)plo;
}
