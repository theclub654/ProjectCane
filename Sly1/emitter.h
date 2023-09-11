#pragma once
#include "xform.h"

struct CRVL;
struct CRVC;

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

class EXPL : public XFM
{
	public:
		class EXPLG *pexplgParent;
};

class EXPLG : public EXPL
{
	public:
		int cpexpl;
		EXPL *apexpl;
};

class EXPLO : public EXPL
{
	public:
		//EMITB *pemitb;
		OID oidreference;
		OID oidShape;
};

void LoadEmitMeshFromBrx(CBinaryInputStream* pbis);
void LoadEmitblipColorsFromBrx(int crgba, CBinaryInputStream* pbis);
void LoadEmitterFromBrx(CBinaryInputStream *pbis);
void LoadExplgFromBrx(CBinaryInputStream *pbis);
void LoadExploFromBrx(CBinaryInputStream *pbis);