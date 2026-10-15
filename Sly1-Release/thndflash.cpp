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
    glm::vec4 LerpRgba(const glm::vec4& rgbaFrom, const glm::vec4& rgbaTo, float u)
    {
        u = std::clamp(u, 0.0f, 1.0f);
        return glm::mix(rgbaFrom, rgbaTo, u);
    }
}

void SetThndFlashRgbaStartFloat(THNDFLASH* pthndflash, const glm::vec4* rgba)
{
    pthndflash->rgbaStart = glm::clamp(*rgba, 0.0f, 1.0f);
}

void SetThndFlashRgbaPeakFloat(THNDFLASH* pthndflash, const glm::vec4* rgba)
{
    pthndflash->rgbaPeak = glm::clamp(*rgba, 0.0f, 1.0f);
}

void SetThndFlashRgbaEndFloat(THNDFLASH* pthndflash, const glm::vec4* rgba)
{
    pthndflash->rgbaEnd = glm::clamp(*rgba, 0.0f, 1.0f);
}

THNDFLASH* NewThndFlash()
{
    return NewWorldObject<THNDFLASH>();
}

int GetThndFlashSize()
{
    return sizeof(THNDFLASH);
}

void CloneThndFlash(THNDFLASH* pthndflash, THNDFLASH* pthndflashBase)
{
    CloneLo(pthndflash, pthndflashBase);

    pthndflash->rgbaStart = pthndflashBase->rgbaStart;
    pthndflash->rgbaPeak = pthndflashBase->rgbaPeak;
    pthndflash->rgbaEnd = pthndflashBase->rgbaEnd;
    pthndflash->dtFadeIn = pthndflashBase->dtFadeIn;
    pthndflash->dtHold = pthndflashBase->dtHold;
    pthndflash->dtFadeOut = pthndflashBase->dtFadeOut;
    pthndflash->thndflashk = pthndflashBase->thndflashk;
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
        pthndflash->rgbaCurrent = LerpRgba(pthndflash->rgbaStart, pthndflash->rgbaPeak, dt / pthndflash->dtFadeIn);
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

            pthndflash->rgbaCurrent = LerpRgba(pthndflash->rgbaPeak, pthndflash->rgbaEnd, dt / pthndflash->dtFadeOut);
        }
    }

    RPL rpl{};
    rpl.rp = pthndflash->thndflashk == 0 ? RP_Background : RP_Foreground;
    rpl.PFNDRAWRPL = DrawThndFlash;
    rpl.pthndflash = pthndflash;
    SubmitRpl(&rpl);
}

void DrawThndFlash(RPL* prpl)
{
    THNDFLASH* pthndflash = prpl->pthndflash;
    const glm::vec4 rgba = glm::clamp(pthndflash->rgbaCurrent, 0.0f, 1.0f) * 255.0f;

    FillScreenRect(static_cast<byte>(rgba.r), static_cast<byte>(rgba.g), static_cast<byte>(rgba.b), static_cast<byte>(rgba.a), 0.0f, 0.0f,
        static_cast<float>(g_gl.width), static_cast<float>(g_gl.height));
}

void DeleteThndFlash(THNDFLASH* pthndflash)
{
    ReleaseWorldObject(pthndflash);
}
