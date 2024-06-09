#include "jump.h"

JMT* NewJmt()
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

void OnJmtRemove(JMT* pjmt)
{
	OnLoRemove(pjmt);
	//RemoveDlEntry(&g_dlJmt, pjmt);
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
