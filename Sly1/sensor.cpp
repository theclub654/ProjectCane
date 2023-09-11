#include "sensor.h"

void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(0, pbis);
}
