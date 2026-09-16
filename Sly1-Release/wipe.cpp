#include "wipe.h"
#include "keyhole.h"
#include "dialog.h"
#include "wm.h"
#include "render.h"
#include "save.h"
#include "binoc.h"

void InitWipe(WIPE* pwipe)
{
	pwipe->wipes = WIPES_Nil;
	SetWipeWipes(pwipe, WIPES_Idle);
}

void UpdateWipe(WIPE* pwipe, JOY* pjoy)
{
    WIPES wipes = pwipe->wipes;
    float dtWipe = g_clock.tReal - pwipe->tWipes;

    if (wipes == WIPES_WipingOut)
    {
        if (pwipe->wipek != WIPEK_WorldMap)
        {
            const float dtWipingOut = pwipe->wipek == WIPEK_Keyhole ? 1.5f : 0.5f;

            if (dtWipe > dtWipingOut)
            {
                wipes = WIPES_Black;
                HandleWipeVolumes(false, 0.0f);
            }
            else
            {
                const float uWipe = glm::clamp(dtWipe / dtWipingOut, 0.0f, 1.0f);

                pwipe->uBlack = uWipe;

                const bool fHandleVolumes = (pwipe->trans.grftrans & 8) == 0;
                HandleWipeVolumes(fHandleVolumes, 1.0f - uWipe);
            }
        }
    }
    else if (wipes == WIPES_Idle)
    {
        if (FCatchWipeButtonTrans(pwipe, pjoy, WIPES_WipingOut))
            return;
    }
    else if (wipes == WIPES_Black)
    {
        if (pwipe->trans.fSet || !g_transition.m_fPending)
            wipes = WIPES_WipingIn;
    }
    else if (wipes == WIPES_WipingIn && pwipe->wipek != WIPEK_WorldMap)
    {
        const float dtWipingIn = pwipe->wipek == WIPEK_Keyhole ? 1.0f : 0.25f;

        if (dtWipe > dtWipingIn)
        {
            if (pwipe->wipek == WIPEK_Frozen)
            {
                pwipe->wipek = WIPEK_Fade;
                pwipe->uBlack = 1.0f;
                pwipe->tWipes = g_clock.tReal;
            }
            else
                wipes = WIPES_Idle;
        }
        else
            pwipe->uBlack = 1.0f - (dtWipe / dtWipingIn);
    }

    SetWipeWipes(pwipe, wipes);
}

void DrawWipe(WIPE* pwipe)
{
    if (pwipe == nullptr || g_psw == nullptr || pwipe->wipes == WIPES_Idle)
        return;

    float uBlack;

    if (pwipe->wipek == WIPEK_Keyhole)
    {
        if (g_pkeyhole != nullptr)
        {
            DrawKeyhole(g_pkeyhole, pwipe->uBlack);
            return;
        }

        // Retail falls back to a black fade when no keyhole is loaded.
        uBlack = pwipe->uBlack;
    }
    else if (pwipe->wipek == WIPEK_Fade)
        uBlack = pwipe->uBlack;
    else
        return;

    const int alpha = static_cast<int>(glm::clamp(uBlack, 0.0f, 1.0f) * 255.0f);

    const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean depthWriteEnabled = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteEnabled);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    FillScreenRect(0, 0, 0, alpha, 0.0f, 0.0f,
                   static_cast<float>(g_gl.width),
                   static_cast<float>(g_gl.height));

    glDepthMask(depthWriteEnabled);
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
}

void ActivateWipe(WIPE* pwipe, TRANS* ptrans, WIPEK wipek)
{
    if (pwipe == nullptr || ptrans == nullptr)
        return;

    if (pwipe->wipes != 0)
        return;

    if (g_binoc.pdialogPlaying != nullptr && g_binoc.pdialogPlaying->dialogk != 2)
        CancelSwDialogPlaying(g_psw);

    bool fSameWorld = false;

    if (ptrans->plevel == nullptr)
        fSameWorld = true;
    else if (ptrans->plevel == g_transition.m_plevelCurrent)
        fSameWorld = true;
    else if (g_transition.m_plevelCurrent != nullptr)
        fSameWorld = ptrans->plevel->searchKey == g_transition.m_plevelCurrent->searchKey;

    constexpr uint32_t GRFTRANS_SameWorld = 0x08;

    uint32_t grftrans =
        static_cast<uint32_t>(ptrans->grftrans);

    if (fSameWorld)
        grftrans |= GRFTRANS_SameWorld;
    else
        grftrans &= ~GRFTRANS_SameWorld;

    ptrans->grftrans = static_cast<GRFTRANS>(grftrans);

    pwipe->trans = *ptrans;
    pwipe->wipek = wipek;

    const int wipes = g_psw != nullptr ? WIPEK_Keyhole : WIPEK_WorldMap;
    SetWipeWipes(pwipe, (WIPES)wipes);
}

void SetWipeButtonTrans(WIPE* pwipe, TRANS* ptrans, WIPEK wipek)
{
    pwipe->transButton = *ptrans;
    pwipe->wipekButton = wipek;
}

int FCatchWipeButtonTrans(WIPE* pwipe, JOY* pjoy, WIPES wipesNew)
{
    if (pwipe == nullptr || pjoy == nullptr)
        return 0;

    if ((g_grfusr & 2U) == 0)
        return 0;

    if (!pwipe->transButton.fSet)
        return 0;

    if (!pjoy->IsPressed(BTN_START))
        return 0;

    pjoy->SetHandled(BTN_START);
    pwipe->trans = pwipe->transButton;
    pwipe->wipek = pwipe->wipekButton;

    const bool fSameWorld =
        pwipe->trans.plevel == nullptr ||
        pwipe->trans.plevel == g_transition.m_plevelCurrent ||
        (g_transition.m_plevelCurrent != nullptr &&
            pwipe->trans.plevel->searchKey == g_transition.m_plevelCurrent->searchKey);

    constexpr uint32_t GRFTRANS_SameWorld = 0x08;

    uint32_t grftrans = static_cast<uint32_t>(pwipe->trans.grftrans);

    if (fSameWorld)
        grftrans |= GRFTRANS_SameWorld;
    else
        grftrans &= ~GRFTRANS_SameWorld;

    pwipe->trans.grftrans = static_cast<GRFTRANS>(grftrans);

    SetWipeWipes(pwipe, wipesNew);
    return 1;
}

constexpr uint32_t GRFTRANS_SameWorld = 0x08;

void SetWipeWipes(WIPE* pwipe, WIPES wipes)
{
    if (pwipe == nullptr)
        return;

    if (pwipe->wipes == wipes)
        return;

    if (pwipe->wipes == static_cast<WIPES>(2) && g_wmc.pwmCurrent != nullptr)
        SetWmWms(g_wmc.pwmCurrent, static_cast<WMS>(0));

    if (wipes == static_cast<WIPES>(1))
    {
        if (pwipe->wipek == WIPEK_WorldMap)
        {
            const uint32_t levelId =
                pwipe->trans.plevel != nullptr
                ? static_cast<uint32_t>(
                    pwipe->trans.plevel->levelID)
                : UINT32_MAX;

            if (g_wmc.pwmCurrent == nullptr || levelId == UINT32_MAX)
                pwipe->wipek = WIPEK_Fade;
            else
            {
                uint32_t worldLevel = 10;

                if ((levelId >> 8) != 0)
                    worldLevel = levelId & 0xFF;

                ShowWm(g_wmc.pwmCurrent, (WORLDLEVEL)worldLevel, WMS_Warping);

                AcquireSwLetterbox(g_psw);
            }
        }
        else if (pwipe->wipek == WIPEK_Frozen)
            wipes = static_cast<WIPES>(2);

        if (wipes == static_cast<WIPES>(1))
        {
            SetUiUis(&g_ui, static_cast<UIS>(8));
            pwipe->wipes = static_cast<WIPES>(1);
            pwipe->tWipes = g_clock.tReal;
            return;
        }
    }

    if (wipes == static_cast<WIPES>(0))
    {
        pwipe->uBlack = 0.0f;
        pwipe->wipes = static_cast<WIPES>(0);
    }
    else if (wipes == static_cast<WIPES>(2))
    {
        if (pwipe->trans.fSet)
            g_transition.Set(pwipe->trans.plevel, pwipe->trans.oidWarp, pwipe->trans.oidWarpContext, pwipe->trans.grftrans);

        pwipe->trans.fSet = 0;
        pwipe->uBlack = 1.0f;
        pwipe->transButton.fSet = 0;
        pwipe->wipes = static_cast<WIPES>(2);
    }
    else if (wipes == static_cast<WIPES>(3))
    {
        if (pwipe->wipek == WIPEK_WorldMap)
        {
            if (g_ui.uisPlaying == 2 &&
                g_gameWorldPrev != GAMEWORLD_Nil)
            {
                g_wmc.pwmCurrent = g_wmc.apwm[g_gameWorldPrev];
            }

            if (g_wmc.pwmCurrent != nullptr)
            {
                g_wmc.pwmCurrent->wmsActive = WMS_Warping;
                SetWmWms(g_wmc.pwmCurrent, static_cast<WMS>(4));
                pwipe->wipes = static_cast<WIPES>(3);
            }
            else
            {
                pwipe->wipek = WIPEK_Frozen;
                pwipe->wipes = static_cast<WIPES>(3);
            }
        }
        else
        {
            if (pwipe->wipek == WIPEK_Frozen)
            {
                FadeFramesToBlack(0.25f);
                pwipe->wipek = WIPEK_Fade;
            }

            pwipe->wipes = static_cast<WIPES>(3);
        }
    }
    else
        pwipe->wipes = wipes;

    pwipe->tWipes = g_clock.tReal;
}

void WipeToWorldWarp(LEVELINFO* plevel, OID oidWarp, WIPEK wipek)
{
    if (plevel == nullptr)
        return;

    TRANS trans{};
    trans.fSet = 1;
    trans.plevel = plevel;
    trans.oidWarp = oidWarp;
    trans.oidWarpContext = OID_Nil;
    trans.grftrans = static_cast<GRFTRANS>(0);

    ActivateWipe(&g_wipe, &trans, wipek);
}

void WipeToWorldWarp(const std::string& levelName, OID oidWarp, WIPEK wipek)
{
    LEVELINFO* plevel = PlevelinfoFromLevelName(levelName);

    if (plevel == nullptr)
        return;

    WipeToWorldWarp(plevel, oidWarp, wipek);
}

void WipeToWorldWarp(const char* pchzLevelName, OID oidWarp, WIPEK wipek)
{
    if (pchzLevelName == nullptr)
        return;

    WipeToWorldWarp(std::string(pchzLevelName), oidWarp, wipek);
}

void FadeFramesToBlack(float dt)
{

}

WIPE g_wipe;
GAMEWORLD g_gameWorldPrev = GAMEWORLD_Nil;
