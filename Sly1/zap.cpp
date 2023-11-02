#include "zap.h"

void InitTzp(TZP* ptzp)
{
	InitSo(ptzp);
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
}
