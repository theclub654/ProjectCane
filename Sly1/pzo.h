#pragma once
#include "po.h"

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

class VAULT : public PO
{
	public:
};

class SCPRIZE : public SPRIZE
{
	public:
};

void InitVault(VAULT* pvault);
void InitSprize(SPRIZE* psprize);
void InitScprize(SCPRIZE* pscprize);
void InitClue(CLUE* pclue);
void LoadSprizeFromBrx(SPRIZE *psprize, CBinaryInputStream *pbis);
void LoadClueFromBrx(CLUE *pclue, CBinaryInputStream *pbis);
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);