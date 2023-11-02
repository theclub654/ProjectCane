#pragma once
#include "po.h"

class JACK : public PO
{
	public:
};
class JACKN : public ALO
{
	public:
};

class JACKB : public ALO
{
	public:
};

class JACKF : public ALO
{
	public:
};

void InitJack(JACK *pjack);
void InitJackb(JACKB* pjackb);
void InitJackn(JACKN* pjackn);
void InitJackf(JACKF* pjackf);
void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis);