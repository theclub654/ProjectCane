#pragma once
#include "so.h"
#include "shape.h"
#include "pnt.h"

class HBSK : public SO
{
	public:
};

class HSHAPE : public SHAPE
{
	public:
};

class HPNT : public PNT
{
	public:
};

void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void InitHshape(HSHAPE* phshape);
void OnHshapeAdd(HSHAPE* phshape);
void InitHpnt(HPNT* phpnt);
void OnHpntAdd(HPNT* phpnt);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
