#include "bsp.h"

void ReadBspc(GEOM *pgeom, BSPC *pbspc, CBinaryInputStream *pbis)
{
    pbspc->cbsp = pbis->U16Read();
    pbspc->absp.resize(pbspc->cbsp);

    pbspc->cbspFull = pbis->U16Read();

    for (int i = 0; i < pbspc->cbspFull; i++)
    {
        pbis->U16Read();
        pbis->U16Read();
        pbis->U16Read();
    }
}
