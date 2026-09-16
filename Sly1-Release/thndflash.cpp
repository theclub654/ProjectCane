#include "thndflash.h"

#include "clock.h"
#include "binoc.h"
#include "glob.h"
#include "gl.h"
#include "render.h"
#include "sw.h"
#include "ui.h"

#include <algorithm>

namespace
{
    RGBA LerpRgba(const RGBA& rgbaFrom, const RGBA& rgbaTo, float u)
    {
        u = std::clamp(u, 0.0f, 1.0f);

        auto lerpByte = [u](byte from, byte to)
        {
            return static_cast<byte>((1.0f - u) * from + u * to);
        };

        return
        {
            lerpByte(rgbaFrom.bRed, rgbaTo.bRed),
            lerpByte(rgbaFrom.bGreen, rgbaTo.bGreen),
            lerpByte(rgbaFrom.bBlue, rgbaTo.bBlue),
            lerpByte(rgbaFrom.bAlpha, rgbaTo.bAlpha)
        };
    }
}

THNDFLASH* NewThndFlash()
{
    return new THNDFLASH{};
}

int GetThndFlashSize()
{
    return sizeof(THNDFLASH);
}

void StartThndFlash(THNDFLASH* pthndflash)
{
    if (pthndflash->fActive == 0)
    {
        pthndflash->fActive = 1;
        pthndflash->tStart = g_clock.t;
        AppendDlEntry(&g_psw->dlThndFlash, pthndflash);
    }
}

void RenderThndFlash(THNDFLASH* pthndflash)
{
    if (g_ui.uis != UIS_Playing || g_binoc.binocs == BINOCS_Peek || g_binoc.binocs == BINOCS_Dialog)
    {
        return;
    }

    float dt = g_clock.t - pthndflash->tStart;

    if (dt < pthndflash->dtFadeIn)
    {
        pthndflash->rgbaCurrent = LerpRgba(
            pthndflash->rgbaStart,
            pthndflash->rgbaPeak,
            dt / pthndflash->dtFadeIn);
    }
    else
    {
        dt -= pthndflash->dtFadeIn;

        if (dt < pthndflash->dtHold)
        {
            // The original interpolates peak to peak during the hold segment.
            pthndflash->rgbaCurrent = pthndflash->rgbaPeak;
        }
        else
        {
            dt -= pthndflash->dtHold;

            if (dt >= pthndflash->dtFadeOut)
            {
                pthndflash->fActive = 0;
                RemoveDlEntry(&g_psw->dlThndFlash, pthndflash);
                return;
            }

            pthndflash->rgbaCurrent = LerpRgba(
                pthndflash->rgbaPeak,
                pthndflash->rgbaEnd,
                dt / pthndflash->dtFadeOut);
        }
    }

    RPL rpl{};
    rpl.rp = RP_Foreground;
    rpl.PFNDRAWRPL = DrawThndFlash;
    rpl.pthndflash = pthndflash;
    SubmitRpl(&rpl);
}

void DrawThndFlash(RPL* prpl)
{
    THNDFLASH* pthndflash = prpl->pthndflash;
    const RGBA& rgba = pthndflash->rgbaCurrent;

    // GS alpha uses 0x80 as fully opaque. Convert that range to the host
    // renderer's conventional 0xff alpha range before drawing.
    const int alpha = std::min(static_cast<int>(rgba.bAlpha) * 2, 255);

    FillScreenRect(rgba.bRed, rgba.bGreen, rgba.bBlue, alpha, 0.0f, 0.0f, static_cast<float>(g_gl.width), static_cast<float>(g_gl.height), pthndflash->thndflashk == 1);
}

void DeleteThndFlash(THNDFLASH* pthndflash)
{
    delete pthndflash;
}
