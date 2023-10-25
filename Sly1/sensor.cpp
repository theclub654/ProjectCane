#include "sensor.h"

void LoadLasenFromBrx(LASEN* plasen, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(plasen, pbis);
}
