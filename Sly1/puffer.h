#pragma once
#include "po.h"
#include "chkpnt.h"

class PUFFER : public PO
{
	public:

};

class PUFFV : public VOL
{
	public:
};

void InitPuffer(PUFFER *ppuffer);
void InitPuffv(PUFFV* ppuffv);
void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream *pbis);