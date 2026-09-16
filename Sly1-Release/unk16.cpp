#include "unk16.h"

void* NewUnk16()
{
    return new UNK16{};
}

void InitUnk16(UNK16* punk16)
{
    InitStepGuard(punk16);
}

void LoadUnk16FromBrx(UNK16* punk16, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(punk16, pbis);
}

void CloneUnk16(UNK16* punk16, UNK16* punk16Base)
{
	CloneLo(punk16, punk16Base);
}

void PostUnk16Load(UNK16* punk16)
{
	PostStepguardLoad(punk16);
}

int GetUnk16Size()
{
	return sizeof(UNK16);
}

void DeleteUnk16(UNK16* punk16)
{
	delete punk16;
}
