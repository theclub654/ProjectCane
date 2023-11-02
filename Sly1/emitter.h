#pragma once
#include "xform.h"

class EMITTER : public ALO
{
	public:
};

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

class EXPLS : public EXPLO
{
	public:
};

void InitExplo(EXPLO* pexplo);
void InitEmitter(EMITTER* pemitter);
void InitExpls(EXPLS* pexpls);
void LoadEmitMeshFromBrx(CBinaryInputStream* pbis);
void LoadEmitblipColorsFromBrx(int crgba, CBinaryInputStream* pbis);
void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis);
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void LoadExploFromBrx(EXPLO *pexplo, CBinaryInputStream *pbis);