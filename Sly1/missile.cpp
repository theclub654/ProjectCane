#include "missile.h"

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis)
{
	LoadBombFromBrx(pmissile, pbis);
}
