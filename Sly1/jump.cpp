#include "jump.h"

void OnJmtAdd(JMT* pjmt)
{
	std::cout << "JMT Size: " << sizeof(JMT) << "\n";
	OnLoAdd(pjmt);
}

void DeleteJmt(LO* plo)
{
	delete(JMT*)plo;
}
