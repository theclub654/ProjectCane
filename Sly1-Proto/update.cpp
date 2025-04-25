#include "update.h"

void UpdateSwDarkening(SW* psw, float dt)
{
    SMP psmp{};

    float gCur = psw->rDarken;
    float gTarget = psw->rDarkenSmooth;
    if (gTarget < gCur) 
    {
        psmp.svFast = 2.0;
        psmp.svSlow = 0.0;
        psmp.dtFast = 0.1;
    }
    else {
        if (gTarget <= gCur) {
            return;
        }
        psmp.svFast = 1.0;
        psmp.svSlow = 0.0;
        psmp.dtFast = 0.1;
    }

    psw->rDarken = GSmooth(gCur, gTarget, dt, &psmp, nullptr);
}
