#include "jump.h"

void* NewJmt()
{
	return new JMT;
}

void OnJmtAdd(JMT* pjmt)
{
	OnLoAdd(pjmt);
}

void DeleteJmt(LO* plo)
{
	delete(JMT*)plo;
}
