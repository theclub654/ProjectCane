#pragma once
#include "po.h"

class UNK1 : public PO
{

};

void*NewUnk1();
void LoadUnk1FromBrx(UNK1* punk1, CBinaryInputStream *pbis);
void InitUnk1(UNK1* punk1);
void CloneUnk1(UNK1* punk1, UNK1* punk1Base);
void DeleteUnk1(UNK1* punk1);