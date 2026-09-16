#pragma once
#include "stepguard.h"

//GOTTA COME BACK TO THIS 
class UNK16 : public STEPGUARD
{
	public:

};

void*NewUnk16();
void InitUnk16(UNK16* punk16);
void LoadUnk16FromBrx(UNK16 *punk16, CBinaryInputStream *pbis);
void CloneUnk16(UNK16* punk16, UNK16* punk16Base);
void PostUnk16Load(UNK16* punk16);
int  GetUnk16Size();
void DeleteUnk16(UNK16* punk16);