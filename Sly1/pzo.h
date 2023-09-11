#pragma once
#include "so.h"

class SPRIZE : public SO
{
	public:
	
};

class CLUE : public SPRIZE
{
	public:
};

class LOCK : public ALO
{
	public:
};

class LOCKG : public ALO
{
	public:
};

void LoadSprizeFromBrx(SPRIZE *psprize, CBinaryInputStream *pbis);
void LoadClueFromBrx(CLUE *pclue, CBinaryInputStream *pbis);
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);