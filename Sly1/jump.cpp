#include "jump.h"

void* NewJmt()
{
	return new JMT{};
}

int GetJmtSize()
{
	return sizeof(JMT);
}

void OnJmtAdd(JMT* pjmt)
{
	OnLoAdd(pjmt);
}

void CloneJmt(JMT* pjmt, JMT* pjmtBase)
{
	LO lo = *pjmt;
	*pjmt = *pjmtBase;
	memcpy(pjmt, &lo, sizeof(LO));

	CloneLo(pjmt, pjmtBase);
}

void DeleteJmt(LO* plo)
{
	delete(JMT*)plo;
}
