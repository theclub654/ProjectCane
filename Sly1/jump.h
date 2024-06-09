#pragma once
#include "pnt.h"

class JMT : public PNT
{
	public:
		DLE dleJmt;
		float sRadius;
		OID oidSetIgnoreJt;
		struct SO* psoSetIgnoreJt;
};

JMT* NewJmt();
int  GetJmtSize();
void OnJmtAdd(JMT* pjmt);
void OnJmtRemove(JMT* pjmt);
void CloneJmt(JMT* pjmt, JMT* pjmtBase);
void DeleteJmt(LO* plo);