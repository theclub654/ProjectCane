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

void* NewJmt();
void OnJmtAdd(JMT* pjmt);
void DeleteJmt(LO* plo);