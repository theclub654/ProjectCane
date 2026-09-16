#include "wm.h"
#include "wipe.h"
#include "totals.h"
#include "pnt.h"
#include "keyhole.h"
#include <cstdio>

WORLDLEVEL WorldLevelForWmDisplay()
{
    if (g_ui.uisPlaying == UIS_Playing)
        return static_cast<WORLDLEVEL>(10);

    return g_pgsCur->worldLevelCur;
}

bool FResolveWmLevel(WM* pwm, WORLDLEVEL worldLevelWm, GAMEWORLD* pgameWorld, WORLDLEVEL* pworldLevel)
{
    GAMEWORLD gameWorld = pwm->gameWorldCur;
    WORLDLEVEL worldLevel = worldLevelWm;
    bool available = false;

    if (worldLevelWm == static_cast <WORLDLEVEL>(10))
    {
        gameWorld = static_cast<GAMEWORLD>(0);
        worldLevel = static_cast<WORLDLEVEL>(4);
        available = true;
    }
    else
    {
        const int gameWorldIndex = static_cast<int>(pwm->gameWorldCur);

        bool valid = true;

        if (gameWorldIndex == 5 && (worldLevelWm == static_cast<WORLDLEVEL>(1) || worldLevelWm == static_cast<WORLDLEVEL>(7)))
            valid = false;

        if (worldLevelWm == static_cast<WORLDLEVEL>(9))
        {
            worldLevel = static_cast<WORLDLEVEL>(1);

            if (valid && (g_pgsCur->aws[gameWorldIndex].als[1].grfls & 1U) != 0)
                available = (g_pgsCur->aws[gameWorldIndex].fws & g_agrfwsSpecialMapUnlock[gameWorldIndex]) != 0;
        }
        else if (worldLevelWm < static_cast<WORLDLEVEL>(9))
        {
            if (valid)
                available = (g_pgsCur->aws[gameWorldIndex].als[worldLevelWm].grfls & 1U) != 0;
        }
        else
            worldLevel = WORLDLEVEL_Nil;
    }

    if (pgameWorld != nullptr)
        *pgameWorld = gameWorld;

    if (pworldLevel != nullptr)
        *pworldLevel = worldLevel;

    return available;
}

bool FIsWmLevelAvailable(WM* pwm, WORLDLEVEL worldLevel)
{
    if (worldLevel == WORLDLEVEL_Max)
        return false;

    if (FResolveWmLevel(pwm, worldLevel, nullptr, nullptr))
        return true;

    WORLDLEVEL fallbackLevel = WORLDLEVEL_Nil;

    switch (pwm->gameWorldCur)
    {
        case GAMEWORLD_Underwater:
        if (worldLevel < WORLDLEVEL_1 || worldLevel > WORLDLEVEL_6)
            return false;

        fallbackLevel = worldLevel < WORLDLEVEL_5 ? WORLDLEVEL_Hub : WORLDLEVEL_Max;
        break;

        case GAMEWORLD_Snow:
        if (worldLevel < WORLDLEVEL_1 || worldLevel > WORLDLEVEL_6)
            return false;

        fallbackLevel = worldLevel < WORLDLEVEL_4 ? WORLDLEVEL_Hub : WORLDLEVEL_Max;
        break;

        default:
        return false;
    }

    return FResolveWmLevel(pwm, fallbackLevel, nullptr, nullptr);
}

WM* NewWm()
{
	return new WM{};
}

int GetWmSize()
{
	return sizeof(WM);
}

void CloneWm(WM* pwm, WM* pwmBase)
{
    CloneAlo(pwm, pwmBase);

    pwm->wms = pwmBase->wms;
    pwm->tWms = pwmBase->tWms;
    pwm->worldlevelCur = pwmBase->worldlevelCur;
    pwm->worldlevelGoal = pwmBase->worldlevelGoal;
    pwm->wmsActive = pwmBase->wmsActive;
    pwm->pasegOpen = pwmBase->pasegOpen;
    pwm->pasegClose = pwmBase->pasegClose;
    pwm->pasegaOpenClose = pwmBase->pasegaOpenClose;
    pwm->pasegaMove = pwmBase->pasegaMove;
    // Clone WMW array
    for (int i = 0; i < 9; i++) {
        pwm->awmw[i] = pwmBase->awmw[i];
    }

    pwm->worldlevelCursor = pwmBase->worldlevelCursor;
    pwm->posCursor = pwmBase->posCursor;
    pwm->posCursorTarget = pwmBase->posCursorTarget;
    pwm->fManualWarp = pwmBase->fManualWarp;
}

void PostWmLoad(WM* pwm)
{
    PostAloLoad(pwm);

    pwm->fNoFreeze = 1;

    // Retail leaves the controller's current map untouched only while the
    // intro world is active. GAMEWORLD_Nil is not the condition used here.
    if (g_pgsCur->gameWorldCur != GAMEWORLD_Intro)
        g_wmc.pwmCurrent = (WM*)pwm;

    switch (pwm->oid)
    {
        case 454:
        pwm->gameWorldCur = GAMEWORLD_Snow;
        break;

        case 455:
        pwm->gameWorldCur = GAMEWORLD_Underwater;
        break;

        case 456:
        pwm->gameWorldCur = GAMEWORLD_Muggshot;
        break;

        case 457:
        pwm->gameWorldCur = GAMEWORLD_Voodoo;
        break;

        case 458:
        pwm->gameWorldCur = GAMEWORLD_Clockwerk;
        break;

        default:
        pwm->gameWorldCur = g_pgsCur->gameWorldCur;
        break;
    }

    g_wmc.apwm[pwm->gameWorldCur] = pwm;
}

void BindWm(WM* pwm)
{
    BindAlo(pwm);

    SnipAloObjects(pwm, 52, s_asnipWm);

    DLI dliChild;
    dliChild.m_pdl = &pwm->dlChild;
    dliChild.m_ibDle = pwm->dlChild.ibDle;
    dliChild.m_pdliNext = s_pdliFirst;

    LO* plo = pwm->dlChild.ploFirst;
    dliChild.m_ppv = plo != nullptr
        ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(plo) + dliChild.m_ibDle)
        : nullptr;
    s_pdliFirst = &dliChild;

    while (plo != nullptr)
    {
        SnipLo(plo);

        // SnipLo may remove the current entry. RemoveDlEntry repairs active
        // DLI walkers, so fetch the next child through the repaired cursor.
        plo = dliChild.m_ppv != nullptr ? static_cast<LO*>(*dliChild.m_ppv) : nullptr;
        dliChild.m_ppv = plo != nullptr
            ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(plo) + dliChild.m_ibDle)
            : nullptr;
    }

    s_pdliFirst = dliChild.m_pdliNext;

    for (int i = 0; i < 11; ++i)
    {
        WMW& wmw = pwm->awmw[i];

        wmw.uDotTarget = 0.0f;
        wmw.uDot = 0.0f;
        wmw.radDot = GRandInRange(0.0f, 6.4f);
        wmw.swDot = GRandInRange(0.5f, 0.7f);
    }

    pwm->worldlevelCursor = WORLDLEVEL_Nil;
}

void RefreshWmMoveStats(WM* pwm, WORLDLEVEL worldLevel)
{
    if (worldLevel == (WORLDLEVEL_Boss | WORLDLEVEL_1))
        SetTotalsLevelText(&g_totals, GAMEWORLD_Intro, WORLDLEVEL_3);
    else
    {
        const WORLDLEVEL statsLevel = worldLevel < WORLDLEVEL_Max ? worldLevel : WORLDLEVEL_Nil;
        SetTotalsLevelText(&g_totals, pwm->gameWorldCur, statsLevel);
    }

    SetWmCursor(pwm, worldLevel);
}

void ThrowWmDisplayState(WM* pwm, WORLDLEVEL worldlevel, int fReverse)
{
    g_worldlevelWmPlayed = worldlevel;
    g_fWmReversePlayed = fReverse;
}

void CatchWmDisplayState(WM* pwm)
{
    if (g_worldlevelWmPlayed == WORLDLEVEL_Nil)
        return;

    const int levelId = (static_cast<int>(g_pgsCur->gameWorldCur) << 8) | static_cast<int>(g_pgsCur->worldLevelCur);
    const float letterboxVisibleTime = levelId == 4 ? 0.5f : 2.0f;

    SetBlotDtVisible(&g_letterbox, letterboxVisibleTime);
    g_letterbox.pvtblot->pfnSetBlotBlots(&g_letterbox, BLOTS_Visible);

    g_totals.fReshow = false;
    g_totals.pvtblot->pfnSetBlotBlots(&g_totals, BLOTS_Hidden);

    SetTotalsLevelText(&g_totals, g_pgsCur->gameWorldCur, g_pgsCur->worldLevelCur);

    g_totals.fDrawOverLetterbox = 1;

    SetBlotDtVisible(&g_totals, 0.0f);
    g_totals.pvtblot->pfnSetBlotBlots(&g_totals, BLOTS_Visible);

    ASEG* pasegWarp = pwm->awmw[g_worldlevelWmPlayed].pasegWarp;

    if (pasegWarp != nullptr)
    {
        const float t = g_fWmReversePlayed ? 0.0f : pasegWarp->tMax;
        ApplyAsegCur(pasegWarp, pwm, t, 0.0f, 0, &pwm->pasegaMove);
    }

    ASEG* pasegClose = pwm->pasegClose;

    if (pasegClose != nullptr)
    {
        for (const EA& ea : pwm->pasegClose->aeaApply)
        {
            if (ea.eatyp != EATYP_HideObject)
                continue;

            LO* plo = PloFindSwObject(pwm->psw, 260, ea.showhide.oid, static_cast<LO*>(pwm));

            if (plo != nullptr)
                plo->pvtlo->pfnAddLo(plo);
        }
    }

    g_worldlevelWmPlayed = WORLDLEVEL_Nil;
}

void UpdateWm(WM* pwm, float dt)
{
    UpdateAlo(pwm, dt);

    if (pwm->wms == WMS_Manual)
    {
        if (pwm->worldlevelCur != pwm->worldlevelGoal)
        {
            RefreshWmMoveStats(pwm, pwm->worldlevelGoal);

            if (FResolveWmLevel(pwm, pwm->worldlevelCur, nullptr, nullptr))
                pwm->awmw[pwm->worldlevelCur].uAlphaTarget = 0.0f;

            if (FResolveWmLevel(pwm, pwm->worldlevelGoal, nullptr, nullptr))
                pwm->awmw[pwm->worldlevelGoal].uAlphaTarget = 1.0f;

            // Retail mirrors the hub fade into the duplicate map layer at index 9.
            pwm->awmw[9].uAlphaTarget = pwm->awmw[1].uAlphaTarget;
            pwm->worldlevelCur = pwm->worldlevelGoal;
        }

        pwm->posCursor = PosSmooth(pwm->posCursor, pwm->posCursorTarget, g_clock.dtReal, &g_smpWmCursor, nullptr);
    }
    else if (pwm->wms == WMS_Hidden)
    {
        if (pwm->pasegaOpenClose != nullptr)
        {
            RetractAsega(pwm->pasegaOpenClose);
            pwm->pasegaOpenClose = nullptr;
        }

        if (pwm->pasegaMove != nullptr)
        {
            RetractAsega(pwm->pasegaMove);
            pwm->pasegaMove = nullptr;
        }

        if (pwm->pasegaOpenClose == nullptr && pwm->pasegaMove == nullptr)
            pwm->pvtlo->pfnRemoveLo(static_cast<LO*>(pwm));
    }
    else if (pwm->wms == WMS_Warping && pwm->worldlevelCur != pwm->worldlevelGoal)
    {
        if (!pwm->fManualWarp)
        {
            RefreshWmMoveStats(pwm, pwm->worldlevelGoal);
            SetBlotDtVisible(static_cast<BLOT*>(&g_totals), 0.0f);
            g_totals.pvtblot->pfnShowBlot(static_cast<BLOT*>(&g_totals));
        }

        const bool fMoveForward = pwm->worldlevelGoal != WORLDLEVEL_Hub || pwm->fManualWarp;

        WORLDLEVEL worldLevelDisplay;

        if (fMoveForward)
            worldLevelDisplay = pwm->worldlevelGoal;
        else
            worldLevelDisplay = WorldLevelForWmDisplay();

        ASEG* pasegWarp = pwm->awmw[worldLevelDisplay].pasegWarp;

        // Retail compares uisPlaying against the raw value 2, which is
        // UIS_Hub. While the world map is being driven from the hub, its
        // existing layer targets must be preserved rather than rewritten.
        if (g_ui.uisPlaying != UIS_Hub)
        {
            const WORLDLEVEL worldLevelPrevious = WorldLevelForWmDisplay();

            pwm->awmw[worldLevelPrevious].uAlphaTarget = 0.0f;
            pwm->awmw[pwm->worldlevelGoal].uAlphaTarget = 1.0f;
            pwm->awmw[pwm->worldlevelGoal].uPrimaryAlphaTarget = 1.0f;
            // Retail mirrors the hub fade into the duplicate map layer at index 9.
            pwm->awmw[9].uAlphaTarget = pwm->awmw[1].uAlphaTarget;
        }

        if (pasegWarp != nullptr)
        {
            const float t = fMoveForward ? pasegWarp->tMax : 0.0f;
            const float svt = fMoveForward ? -1.0f : 1.0f;

            ApplyAsegCur(pasegWarp, static_cast<ALO*>(pwm), t, svt, 1, &pwm->pasegaMove);
            ThrowWmDisplayState(pwm, worldLevelDisplay, fMoveForward);
        }

        pwm->fManualWarp = false;
        pwm->worldlevelCur = pwm->worldlevelGoal;
    }

    if (pwm->wms == WMS_Hidden)
        return;

    SMP* psmpAlpha = pwm->wmsActive == WMS_Manual ? &g_smpWmAlphaFast : &g_smpWmAlphaSlow;

    for (WMW& wmw : pwm->awmw)
    {
        wmw.uDot = GSmooth(wmw.uDot, wmw.uDotTarget, g_clock.dtReal, &g_smpWmDot, nullptr);
        wmw.radDot = RadNormalize(wmw.radDot + wmw.swDot * g_clock.dtReal);

        if (wmw.paloPrimary != nullptr)
        {
            EnsureAloFader(wmw.paloPrimary);
            wmw.paloPrimary->pfader->uAlpha = GSmooth(wmw.paloPrimary->pfader->uAlpha, wmw.uPrimaryAlphaTarget, dt, psmpAlpha, nullptr);
        }

        if (wmw.paloSecondary != nullptr)
        {
            EnsureAloFader(wmw.paloSecondary);
            wmw.paloSecondary->pfader->uAlpha = GSmooth(wmw.paloSecondary->pfader->uAlpha, wmw.uAlphaTarget, dt, psmpAlpha, nullptr);
        }
    }

}

static float WmFullscreenCoverScale()
{
    constexpr float kWmAspect = 640.0f / 492.80002f;
    const float screenHeight = g_gl.height > 0.0f
        ? static_cast<float>(g_gl.height)
        : 1.0f;
    const float screenAspect = static_cast<float>(g_gl.width) / screenHeight;

    return std::max(1.0f, screenAspect / kWmAspect);
}

static glm::vec2 WmFullscreenScreenPoint(float x, float y)
{
    const float scale = WmFullscreenCoverScale();
    const glm::vec2 center(static_cast<float>(g_gl.width) * 0.5f,
                           static_cast<float>(g_gl.height) * 0.5f);

    return glm::vec2(center.x + (x - center.x) * scale, y);
}

void RenderWmAll(WM* pwm, CM* pcm, RO* pro)
{
    RO ro;

    DupAloRo(pwm, pro, &ro);
    LoadMatrixFromPosRot(&pcm->pos, &pcm->mat, &ro.model);

    const float coverScale = WmFullscreenCoverScale();
    const int worldMapFirst = g_worldMapCount;

    RenderAloAll(pwm, pcm, &ro);

    // RenderAloAll queues the map for DrawSw rather than drawing immediately.
    // Conjugating the stretch through the camera matrix makes it operate on
    // clip-space X, independent of each map object's local orientation.
    if (coverScale > 1.0f)
    {
        glm::mat4 clipScale(1.0f);
        clipScale[0][0] = coverScale;

        const glm::mat4 worldToClipInverse = glm::inverse(pcm->matWorldToClip);
        const glm::mat4 worldScreenStretch =
            worldToClipInverse * clipScale * pcm->matWorldToClip;

        for (int i = worldMapFirst; i < g_worldMapCount; ++i)
            g_worldMapPrpl[i].ro.model = worldScreenStretch * g_worldMapPrpl[i].ro.model;
    }
}

void HandleWmMessage(WM* pwm, MSGID msgid, void* pv)
{
    ASEGA* pasega = static_cast<ASEGA*>(pv);
    HandleAloMessage(pwm, msgid, pasega);

    if (msgid != MSGID_asega_limit)
        return;

    switch (pwm->wms)
    {
        case WMS_Warping:
        SetWipeWipes(&g_wipe, WIPES_Black);
        break;

        case WMS_Appearing:
        SetWmWms(pwm, pwm->wmsActive);
        break;

        case WMS_Disappearing:
        if (pasega != pwm->pasegaOpenClose)
            break;

        if (g_wipe.wipes == WIPES_WipingIn)
            SetWipeWipes(&g_wipe, WIPES_Idle);

        SetWmWms(pwm, WMS_Hidden);
        break;

        default:
        break;
    }
}

void SetWmWms(WM* pwm, WMS wms)
{
    if (pwm->wms == wms)
        return;

    const WMS wmsOld = pwm->wms;

    if (wmsOld == WMS_Appearing || wmsOld == WMS_Disappearing)
        StopSound(pwm->pambWmc, 0);
    else if (wmsOld == WMS_Hidden)
    {
        pwm->pvtlo->pfnAddLo(pwm);

        for (int i = 0; i < 11; ++i)
        {
            WMW& wmw = pwm->awmw[i];
            const WORLDLEVEL worldLevel = static_cast<WORLDLEVEL>(i);

            if (!FResolveWmLevel(pwm, worldLevel, nullptr, nullptr))
            {
                wmw.uPrimaryAlphaTarget = 0.0f;
                wmw.uAlphaTarget = 0.0f;
            }
            else
            {
                wmw.uPrimaryAlphaTarget = 1.0f;

                const WORLDLEVEL worldLevelActive =
                    pwm->wmsActive == WMS_Warping
                    ? WorldLevelForWmDisplay()
                    : pwm->worldlevelGoal;

                wmw.uAlphaTarget = worldLevel == worldLevelActive ? 1.0f : 0.0f;
            }
        }

        pwm->awmw[9].uAlphaTarget = pwm->awmw[1].uAlphaTarget;

        for (WMW& wmw : pwm->awmw)
        {
            if (wmw.paloPrimary != nullptr)
            {
                wmw.paloPrimary->pvtlo->pfnAddLo(wmw.paloPrimary);
                EnsureAloFader(wmw.paloPrimary);

                wmw.paloPrimary->pfader->duAlpha = 0.0f;
                wmw.paloPrimary->pfader->uAlpha = wmw.uPrimaryAlphaTarget;
            }

            if (wmw.paloSecondary != nullptr)
            {
                wmw.paloSecondary->pvtlo->pfnAddLo(wmw.paloSecondary);
                EnsureAloFader(wmw.paloSecondary);

                wmw.paloSecondary->pfader->duAlpha = 0.0f;
                wmw.paloSecondary->pfader->uAlpha = wmw.uAlphaTarget;
            }
        }
    }
    else if (wmsOld == WMS_Manual)
    {
        if (!pwm->fManualWarp)
            g_totals.pvtblot->pfnHideBlot(&g_totals);

        for (WMW& wmw : pwm->awmw)
            wmw.uDotTarget = 0.0f;
    }

    switch (wms)
    {
        case WMS_Hidden:
        {
            for (WMW& wmw : pwm->awmw)
            {
                ALO* apalo[] = { wmw.paloPrimary, wmw.paloSecondary };

                for (ALO* palo : apalo)
                {
                    if (palo == nullptr)
                        continue;

                    if (palo->pfader != nullptr)
                        RemoveFader(palo->pfader.get());

                    palo->pvtlo->pfnRemoveLo(palo);
                }
            }

            pwm->wms = WMS_Hidden;
            break;
        }

        case WMS_Appearing:
        {
            WORLDLEVEL worldLevelCurrent;

            if (pwm->wmsActive == WMS_Warping)
            {
                worldLevelCurrent = pwm->worldlevelGoal;
                const bool fHub = worldLevelCurrent == WORLDLEVEL_Hub;

                const WORLDLEVEL worldLevelAnimation =
                    fHub ? WorldLevelForWmDisplay() : worldLevelCurrent;

                ASEG* pasegWarp = pwm->awmw[worldLevelAnimation].pasegWarp;

                if (pasegWarp != nullptr)
                {
                    const float t = fHub ? 0.0f : pasegWarp->tMax;
                    ApplyAsegCur(pasegWarp, pwm, t, 0.0f, 0, &pwm->pasegaMove);
                }

                worldLevelCurrent = pwm->worldlevelGoal;
            }
            else
            {
                ASEG* pasegWarp = pwm->awmw[1].pasegWarp;

                if (pasegWarp != nullptr)
                    ApplyAsegCur(pasegWarp, pwm, 0.0f, 0.0f, 0, &pwm->pasegaMove);

                SetWmCursor(pwm, pwm->worldlevelGoal);
                pwm->posCursor = pwm->posCursorTarget;

                RefreshWmMoveStats(pwm, pwm->worldlevelGoal);
                SetBlotDtVisible(&g_totals, 0.0f);
                g_totals.pvtblot->pfnShowBlot(&g_totals);

                StartSound((SFXID)138, &pwm->pambWmc, nullptr, nullptr, 1000.0f, 200.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

                for (int i = 0; i < 11; ++i)
                {
                    WMW& wmw = pwm->awmw[i];

                    // Retail stores availability in fNav. There is no separate
                    // fAvailable member in its 52-byte WMW structure.
                    wmw.fNav = FIsWmLevelAvailable(pwm, static_cast<WORLDLEVEL>(i));
                    wmw.fDot = wmw.fNav && (i == 0 || (i >= 2 && i <= 7));

                    wmw.uDot = 0.0f;
                    wmw.uDotTarget = 0.0f;
                }

                worldLevelCurrent = pwm->worldlevelGoal;
            }

            pwm->worldlevelCur = worldLevelCurrent;

            if (pwm->pasegOpen != nullptr)
            {
                ApplyAsegCur(pwm->pasegOpen, static_cast<ALO*>(pwm), 0.0f, 1.0f, 0, &pwm->pasegaOpenClose);
                pwm->wms = wms;
            }
            else
            {
                wms = WMS_Hidden;
                pwm->wms = wms;
            }

            break;
        }

        case WMS_Manual:
        {
            for (int i = 0; i < 11; ++i)
            {
                if (i != static_cast<int>(pwm->worldlevelCursor))
                    pwm->awmw[i].uDotTarget = 1.0f;
            }

            pwm->wms = wms;
            break;
        }

        case WMS_Warping:
        {
            pwm->worldlevelCur = WORLDLEVEL_Nil;
            pwm->wms = wms;
            break;
        }

        case WMS_Disappearing:
        {
            if (!pwm->fManualWarp)
            {
                g_totals.pvtblot->pfnHideBlot(&g_totals);

                if (pwm->wmsActive == WMS_Manual)
                    StartSound((SFXID)139, &pwm->pambWmc, nullptr, nullptr, 1000.0f, 200.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            }

            CatchWmDisplayState(pwm);

            if (pwm->pasegClose != nullptr)
            {
                ApplyAsegCur(pwm->pasegClose, pwm, 0.0f, 1.0f, 0, &pwm->pasegaOpenClose);
                pwm->wms = wms;
            }
            else
            {
                wms = WMS_Hidden;
                pwm->wms = wms;
            }

            break;
        }

        default:
        {
            pwm->wms = wms;
            break;
        }
    }

    pwm->tWms = g_clock.tReal;
}

void ShowWm(WM* pwm, WORLDLEVEL worldLevel, WMS wmsActive)
{
    if (pwm == nullptr)
        return;

    if (pwm->wms == WMS_Manual)
    {
        pwm->worldlevelGoal = worldLevel;

        if (wmsActive == WMS_Warping)
            SetWmWms(pwm, WMS_Warping);

        return;
    }

    if (pwm->wms != WMS_Hidden && pwm->wms != WMS_Disappearing)
        return;

    pwm->worldlevelGoal = worldLevel;
    pwm->wmsActive = wmsActive;

    SetWmWms(pwm, WMS_Appearing);
}

void HideWm(WM* pwm)
{
    if ((pwm->wms < WMS_Warping) && (WMS_Hidden < pwm->wms)) 
        SetWmWms(pwm, WMS_Disappearing);
}

void MoveWm(WM* pwm, WMD wmd)
{
    const WmNavTable* pnav = s_mpaaworldmapNav[pwm->gameWorldCur];

    if (pnav == nullptr)
    {
        StartSound((SFXID)123, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        return;
    }

    WORLDLEVEL worldLevel = (*pnav)[pwm->worldlevelGoal][wmd];

    while (worldLevel != WORLDLEVEL_Nil && !pwm->awmw[worldLevel].fNav)
        worldLevel = (*pnav)[worldLevel][wmd];

    if (worldLevel != WORLDLEVEL_Nil)
    {
        ShowWm(pwm, worldLevel, WMS_Manual);
        StartSound(SFXID(121), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        return;
    }

    StartSound((SFXID)123, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void GetWmWorldPosScreen(WM* pwm, WORLDLEVEL worldLevel, bool fSecondary, glm::vec3* pposScreen)
{
    const float screenWidth  = g_gl.width;
    const float screenHeight = g_gl.height;

    const WMW& wmw = pwm->awmw[worldLevel];
    PNT* ppnt = fSecondary ? wmw.ppntSecondary : wmw.ppntPrimary;

    if (ppnt == nullptr)
    {
        *pposScreen = glm::vec3(screenWidth * 0.5f, screenHeight * 0.5f, 0.0f);
        return;
    }

    glm::vec3 posCameraLocal{};
    ConvertAloPos(ppnt->paloParent, pwm, &ppnt->posLocal, &posCameraLocal);

    const glm::vec3 posWorld = g_pcm->mat * posCameraLocal + g_pcm->pos;

    ConvertCmWorldToScreen(g_pcm, &posWorld, pposScreen);

    pposScreen->x = (pposScreen->x * 0.5f  + 0.5f) * screenWidth;
    pposScreen->y = (-pposScreen->y * 0.5f + 0.5f) * screenHeight;

    const glm::vec2 fullscreenPoint = WmFullscreenScreenPoint(pposScreen->x, pposScreen->y);
    pposScreen->x = fullscreenPoint.x;
    pposScreen->y = fullscreenPoint.y;
}

void SetWmCursor(WM* pwm, WORLDLEVEL worldlevel)
{
    if (pwm->worldlevelCursor == worldlevel)
        return;

    const WMW& wmw = pwm->awmw[worldlevel];

    if (wmw.ppntPrimary == nullptr)
        pwm->posCursorTarget = glm::vec3(0.0f);
    else
        ConvertAloPos(wmw.ppntPrimary->paloParent, pwm, &wmw.ppntPrimary->posLocal, &pwm->posCursorTarget);

    // Hide the dot belonging to the newly selected level.
    pwm->awmw[worldlevel].uDotTarget = 0.0f;

    // Restore the dot belonging to the previously selected level.
    if (pwm->worldlevelCursor != WORLDLEVEL_Nil)
        pwm->awmw[pwm->worldlevelCursor].uDotTarget = 1.0f;

    pwm->worldlevelCursor = worldlevel;
}

void DeleteWm(WM* pwm)
{
	delete pwm;
}

void StartupWmc(WMC* pwmc)
{
    // Release static initializer (FUN_001f2688).  Reproduce its fields
    // explicitly because CTextEdge is not the PS2's packed 28-byte layout.
    g_teWmc.m_pfont = nullptr;
    g_teWmc.m_rgba = glm::vec4(0.0f, 75.0f / 255.0f, 125.0f / 255.0f, 1.0f);
    g_teWmc.m_ch = '='; // DAT_00276624 = 61
    g_teWmc.m_dxExtra = 2.0f;
    g_teWmc.m_dyExtra = 1.0f;
    g_teWmc.m_rxScaling = 0.3f;
    g_teWmc.m_ryScaling = 0.3f;

    g_rgbaBoC = glm::vec4(0.0f);
    g_rgbaBoE = glm::vec4(0.0f);
    g_sWmDotInner = g_sWmDotOuter * 0.75f;

    pwmc->pvtwmc = &g_vtwmc;
}

void PostWmcLoad(WMC* pwmc)
{
    // Initialize BLOT base
    PostBlotLoad(pwmc);

    // Assign font for text edge rendering
    if (FFontLoaded(2))
    {
        pwmc->pte = &g_teWmc;
        g_teWmc.m_pfont = PfontFromFont(2);
    }
    
    // Set initial visibility to hidden (uDt = 0.0)
    SetBlotDtVisible(pwmc, 0.0f);

    // Set font scale to 0.6x
    SetBlotFontScale(pwmc, 0.6f);

    pwmc->rgba = glm::vec4(0.498f, 0.498f, 0.498f, 0.874f);

    // Set default draw string via virtual call
    if (pwmc->pvtblot && pwmc->pvtblot->pfnSetBlotAchzDraw)
        pwmc->pvtblot->pfnSetBlotAchzDraw(pwmc, (char*)"&2T&.: Exit\n&2(&.: Move\n&2X&.: Warp"); // replace with real string if needed

    // Null out WM pointer
    pwmc->pwmCurrent = nullptr;

    glGenVertexArrays(1, &pwmc->gaoWmFan);
    glGenBuffers(1, &pwmc->gboWmFan);

    glBindVertexArray(pwmc->gaoWmFan);
    glBindBuffer(GL_ARRAY_BUFFER, pwmc->gboWmFan);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WMFANVERTEX), reinterpret_cast<void*>(offsetof(WMFANVERTEX, pos)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WMFANVERTEX), reinterpret_cast<void*>(offsetof(WMFANVERTEX, uv)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(WMFANVERTEX), reinterpret_cast<void*>(offsetof(WMFANVERTEX, rgba)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OnWmcActive(WMC* pwmc, int fActive)
{
    const bool activate = fActive != 0;

    if (activate == (pwmc->fActive != 0))
        return;

    WM* pwm = pwmc->pwmCurrent;

    if (pwm == nullptr)
        return;

    if (!activate)
    {
        pwmc->pvtblot->pfnHideBlot(pwmc);

        if (!pwm->fManualWarp)
        {
            HideWm(pwmc->pwmCurrent);

            if (pwm->pasegClose != nullptr)
                SetBlotDtDisappear(pwmc, pwm->pasegClose->tMax);

            RemoveGrfusr(1);
            SetUiUis(&g_ui, (UIS)3);

            if ((g_pgsCur->gameWorldCur << 8 | g_pgsCur->worldLevelCur) == 776)
            {
                ResumeVag();
                ContinueMusicSequencer();
            }
        }

        pwmc->fActive = false;
        return;
    }

    ForceHideBlots();
   
    WORLDLEVEL worldLevel = WorldLevelForWmDisplay();

    if (worldLevel == static_cast<WORLDLEVEL>(10))
    {
        // Retail passes null output pointers here and uses the boolean result as
        // WORLDLEVEL_Approach (0) or WORLDLEVEL_Hub (1).
        worldLevel = FResolveWmLevel(pwm, WORLDLEVEL_Hub, nullptr, nullptr)
            ? WORLDLEVEL_Hub
            : WORLDLEVEL_Approach;
    }

    ShowWm(pwm, worldLevel, WMS_Manual);

    pwmc->uWarpTarget = 1.0f;
    pwmc->uWarpTargetGoal = 1.0f;

    if (pwm->pasegOpen != nullptr)
        SetBlotDtAppear(pwmc, pwm->pasegOpen->tMax);

    if ((g_pgsCur->gameWorldCur << 8 | g_pgsCur->worldLevelCur) == 776)
    {
        PauseVag();
        PauseMusicSequencer();
    }

    pwmc->pvtblot->pfnShowBlot(pwmc);

    AddGrfusr(1);
    g_joy.StartJoySelection();
    SetUiUis(&g_ui, (UIS)7);

    pwmc->fActive = true;
}

void UpdateWmcActive(WMC* pwmc, JOY* pjoy)
{
    WM* pwm = pwmc->pwmCurrent;

    if (pwm == nullptr || pwm->wms != WMS_Manual || pwm->fManualWarp)
        return;

    // Release mask 2320 maps to the menu-exit controls in the PC JOY layer.
    const bool fExitPressed = pjoy->IsPressed(BTN_TRIANGLE) ||
                              pjoy->IsPressed(BTN_SELECT) ||
                              pjoy->IsPressed(BTN_START);
    if (fExitPressed || pjoy->joys == JOYS_Searching)
    {
        PopUiActiveBlot(&g_ui);
        pjoy->SetHandled(BTN_TRIANGLE);
        pjoy->SetHandled(BTN_SELECT);
        pjoy->SetHandled(BTN_START);
        return;
    }

    if (pwmc->blots != static_cast<BLOTS>(2))
        return;

    GAMEWORLD gameworld{};
    WORLDLEVEL worldlevel{};

    const bool fCanWarp = FResolveWmLevel(pwm, pwm->worldlevelGoal, &gameworld, &worldlevel);

    const int dx = pjoy->DxSelectionJoy(g_clock.tReal);
    const int dy = pjoy->DySelectionJoy(g_clock.tReal);

    // Retail resets both navigation repeat latches every active update.
    pjoy->tLatchX = g_clock.tReal + pjoy->dtLatchX;
    pjoy->tLatchY = g_clock.tReal + pjoy->dtLatchY;

    if (dy < 0)
        MoveWm(pwm, WMD_Up);
    else if (dy > 0)
        MoveWm(pwm, WMD_Down);
    else if (dx < 0)
        MoveWm(pwm, WMD_Left);
    else if (dx > 0)
        MoveWm(pwm, WMD_Right);
    else if (pjoy->IsPressed(BTN_CROSS))
    {
        pjoy->SetHandled(BTN_CROSS);

        if (pwm->worldlevelGoal == WorldLevelForWmDisplay())
            PopUiActiveBlot(&g_ui);
        else if (!fCanWarp)
            StartSound(static_cast<SFXID>(123), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        else
        {
            const int levelId = (static_cast<int>(gameworld) << 8) | static_cast<int>(worldlevel);
            LEVELINFO* plevel = GetLevelInfo(levelId);

            if (plevel != nullptr)
            {
                pwm->fManualWarp = true;
                PopUiActiveBlot(&g_ui);
                WipeToWorldWarp(plevel, OID_Nil, WIPEK_WorldMap);
            }
        }
    }

    pwmc->uWarpTargetGoal = fCanWarp ? 1.0f : 0.5f;
    pwmc->uWarpTarget = GSmooth(pwmc->uWarpTarget, pwmc->uWarpTargetGoal, g_clock.dtReal, &g_smpWmAlphaFast, nullptr);
}

void DrawWmc(WMC* pwmc)
{
    WM* pwm = pwmc->pwmCurrent;

    if (pwm == nullptr)
        return;

    if (g_pkeyhole != nullptr)
    {
        const float uCursor = pwmc->uOn * pwmc->uOn;

        glm::vec3 posCursorWorld = g_pcm->pos + g_pcm->mat * pwm->posCursor;
        glm::vec3 posCursorScreen{};

        ConvertCmWorldToScreen(g_pcm, &posCursorWorld, &posCursorScreen);

        const float xCursorRaw = (posCursorScreen.x * 0.5f + 0.5f) * static_cast<float>(g_gl.width);
        const float yCursorRaw = (-posCursorScreen.y * 0.5f + 0.5f) * static_cast<float>(g_gl.height);
        const glm::vec2 cursorPoint = WmFullscreenScreenPoint(xCursorRaw, yCursorRaw);
        const float xCursor = cursorPoint.x;
        const float yCursor = cursorPoint.y;

        glm::vec4 rgbaCursorStart(0.0f, 0.0f, 0.0f, (145.0f / 255.0f) * uCursor);
        glm::vec4 rgbaCursorEnd(0.0f);

        DrawWmFan(pwmc, xCursor, yCursor, 40.0f, RadNormalize(g_clock.tReal), 12, &rgbaCursorStart, rgbaCursorEnd, 2);

        const float rKeyholeScale = sinf(g_clock.tReal * 10.0f) * 0.2f + 1.0f;

        DrawKeyholeMask(g_pkeyhole, xCursor, yCursor, rKeyholeScale, uCursor);
    }

    for (int iworldlevel = static_cast<int>(WORLDLEVEL_Approach); iworldlevel < 11; ++iworldlevel)
    {
        const WORLDLEVEL worldlevel = static_cast<WORLDLEVEL>(iworldlevel);
        WMW& wmw = pwm->awmw[iworldlevel];

        if (!wmw.fNav)
            continue;

        glm::vec4 rgbaFanOuter;
        glm::vec4 rgbaFanInner;
        constexpr float sGsAlpha = 75.0f / 128.0f;

        if (wmw.fDot)
        {
            rgbaFanOuter = glm::vec4(159.0f / 255.0f, 1.0f, 232.0f / 255.0f, sGsAlpha);
            rgbaFanInner = glm::vec4(92.0f / 255.0f, 1.0f, 182.0f / 255.0f, 0.0f);
        }
        else
        {
            rgbaFanOuter = glm::vec4(232.0f / 255.0f, 232.0f / 255.0f, 232.0f / 255.0f, sGsAlpha);
            rgbaFanInner = glm::vec4(182.0f / 255.0f, 182.0f / 255.0f, 182.0f / 255.0f, 0.0f);
        }

        glm::vec3 posScreen{};

        GetWmWorldPosScreen(pwm, worldlevel, 0, &posScreen);

        float uDot = wmw.uDot * pwmc->uOn;

        if (FFloatsNear(uDot, 1.0f, 0.0001f) && worldlevel < WORLDLEVEL_Max)
        {
            const GAMEWORLD gameWorld = pwm->gameWorldCur;
            const int levelId = (static_cast<int>(gameWorld) << 8) | iworldlevel;
            const uint32_t tasks = TasksFromLevelID(levelId);
            const uint32_t levelFlags = g_pgsCur->aws[gameWorld].als[iworldlevel].grfls;

            if ((tasks & 2) != 0 && (levelFlags & 2) == 0)
                uDot *= sinf(g_clock.tReal * 20.0f) * 0.2f + 0.8f;
        }

        DrawWmFan(pwmc, posScreen.x, posScreen.y, uDot * g_sWmDotOuter,  wmw.radDot, 14, &rgbaFanOuter, rgbaFanInner, 3);
        DrawWmFan(pwmc, posScreen.x, posScreen.y, uDot * g_sWmDotInner, -wmw.radDot, 10, &rgbaFanOuter, rgbaFanInner, 3);

        if (worldlevel < WORLDLEVEL_Max)
        {
            const GAMEWORLD gameWorld = pwm->gameWorldCur;
            const int levelId = (static_cast<int>(gameWorld) << 8) | iworldlevel;
            const uint32_t tasks = TasksFromLevelID(levelId);

            if ((tasks & 2) != 0)
            {
                const uint32_t levelFlags = g_pgsCur->aws[gameWorld].als[iworldlevel].grfls;
                const bool fTaskComplete = (levelFlags & 2) != 0;

                CFontBrx* pfont = PfontFromFont(3);
                const float rScale = g_rWmTaskMarkerScale * wmw.uDot;

                // A zero-size marker is invisible.  More importantly, putting a
                // zero on CFontBrx's ratio-based scale stack makes PopScaling()
                // divide by zero and poisons this shared font for later rich text.
                if (pfont == nullptr || rScale <= 0.0f)
                    continue;

                pfont->PushScaling(rScale, rScale);

                CTextBox tbx;

                tbx.SetPos(posScreen.x, posScreen.y);
                tbx.SetSize(0.0f, 0.0f);
                tbx.SetTextColor(&pwmc->rgba);
                tbx.SetHorizontalJust(JH_Center);
                tbx.SetVerticalJust(JV_Center);

                const char* pchzMarker = fTaskComplete ? g_pchzWmTaskComplete : g_pchzWmTaskIncomplete;

                pfont->DrawPchz((char*)pchzMarker, &tbx);
                pfont->PopScaling();
            }
        }
    }

}

void DrawWmFan(WMC *pwmc, float xCenter, float yCenter, float sRadius, float rad, int cseg, glm::vec4* rgbaCenter, glm::vec4 rgbaEdge, int grfds)
{
    if (rgbaCenter == nullptr || cseg < 3 || sRadius <= 0.0f)
        return;

    std::vector <WMFANVERTEX> vertices;
    vertices.reserve(static_cast<size_t>(cseg) + 2);

    vertices.push_back({glm::vec2(xCenter, yCenter), glm::vec2(0.5f, 0.5f), *rgbaCenter});

    const float radStep = glm::two_pi<float>() / static_cast<float>(cseg);

    for (int iseg = 0; iseg <= cseg; ++iseg)
    {
        const float angle = rad + static_cast<float>(iseg) * radStep;
        const float xEdge = xCenter + cosf(angle) * sRadius;
        const float yEdge = yCenter + sinf(angle) * sRadius;

        glm::vec4 rgba = rgbaEdge;

        if ((grfds & 2) != 0 && (iseg & 1) != 0)
            rgba = glm::vec4(0.0f);

        vertices.push_back({glm::vec2(xEdge, yEdge), glm::vec2(cosf(angle) * 0.5f + 0.5f, sinf(angle) * 0.5f + 0.5f), rgba});
    }

    glBlotShader.Use();

    const glm::mat4 model(1.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    const glm::vec4 color(1.0f);

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(uvRectLoc, 1, glm::value_ptr(uvRect));
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
    glUniform1i(u_useVertexColorLoc, 1);
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);

    // Retail DrawWmFan writes TEST_1 = 0x31001 (ZTE=1, ZTST=ALWAYS).
    // DrawWmc is layered after the 3D world map, so inheriting the map's
    // depth comparison would allow its geometry to reject the fan overlay.
    const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLint previousDepthFunc = GL_LESS;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_BLEND);

    if ((grfds & 1) != 0)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(pwmc->gaoWmFan);
    glBindBuffer(GL_ARRAY_BUFFER, pwmc->gboWmFan);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(WMFANVERTEX), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(vertices.size()));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDepthFunc(static_cast<GLenum>(previousDepthFunc));
    if (depthTestEnabled == GL_FALSE)
        glDisable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(u_useVertexColorLoc, 0);
}

WMC g_wmc;
CTextEdge g_teWmc;
glm::vec4 g_rgbaBoC;
glm::vec4 g_rgbaBoE;
WORLDLEVEL g_worldlevelWmPlayed = WORLDLEVEL_Nil;
int g_fWmReversePlayed = 0;
int g_agrfwsSpecialMapUnlock[6] = 
{
    0, 2, 4,
    4, 4, 0
};

SMP g_smpWmAlphaSlow = {1.0, 0.0, 0.2};
SMP g_smpWmAlphaFast = {2.0, 0.0, 0.1};
SMP g_smpWmDot = {2.0, 0.0, 0.1};
SMP g_smpWmCursor = {600, 0.0, 0.25};
const WmNavTable s_aaworldmapNavUnderwater =
{
    { (WORLDLEVEL)1,  (WORLDLEVEL)10, (WORLDLEVEL)10, (WORLDLEVEL)5 },
    { (WORLDLEVEL)2,  (WORLDLEVEL)0,  (WORLDLEVEL)3,  (WORLDLEVEL)5 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)1,  (WORLDLEVEL)3,  (WORLDLEVEL)5 },
    { (WORLDLEVEL)2,  (WORLDLEVEL)10, (WORLDLEVEL)-1, (WORLDLEVEL)1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)-1, (WORLDLEVEL)5,  (WORLDLEVEL)7 },
    { (WORLDLEVEL)2,  (WORLDLEVEL)0,  (WORLDLEVEL)1,  (WORLDLEVEL)4 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)7,  (WORLDLEVEL)7,  (WORLDLEVEL)8 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)8,  (WORLDLEVEL)4,  (WORLDLEVEL)6 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)-1, (WORLDLEVEL)7,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)0,  (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)0 }
};

const WmNavTable s_aaworldmapNavMuggshot =
{
    { (WORLDLEVEL)4,  (WORLDLEVEL)10, (WORLDLEVEL)4,  (WORLDLEVEL)10 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)0,  (WORLDLEVEL)4,  (WORLDLEVEL)3 },
    { (WORLDLEVEL)5,  (WORLDLEVEL)3,  (WORLDLEVEL)1,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)2,  (WORLDLEVEL)10, (WORLDLEVEL)1,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)7,  (WORLDLEVEL)0,  (WORLDLEVEL)-1, (WORLDLEVEL)1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)2,  (WORLDLEVEL)8,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)7,  (WORLDLEVEL)-1, (WORLDLEVEL)8 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)4,  (WORLDLEVEL)-1, (WORLDLEVEL)1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)1,  (WORLDLEVEL)6,  (WORLDLEVEL)5 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)3,  (WORLDLEVEL)-1, (WORLDLEVEL)0,  (WORLDLEVEL)-1 }
};

const WmNavTable s_aaworldmapNavVoodoo =
{
    { (WORLDLEVEL)1,  (WORLDLEVEL)10, (WORLDLEVEL)10, (WORLDLEVEL)1 },
    { (WORLDLEVEL)3,  (WORLDLEVEL)0,  (WORLDLEVEL)7,  (WORLDLEVEL)2 },
    { (WORLDLEVEL)4,  (WORLDLEVEL)0,  (WORLDLEVEL)1,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)1,  (WORLDLEVEL)1,  (WORLDLEVEL)4 },
    { (WORLDLEVEL)3,  (WORLDLEVEL)2,  (WORLDLEVEL)1,  (WORLDLEVEL)2 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)10, (WORLDLEVEL)-1, (WORLDLEVEL)8 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)5,  (WORLDLEVEL)-1, (WORLDLEVEL)8 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)0,  (WORLDLEVEL)8,  (WORLDLEVEL)1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)7,  (WORLDLEVEL)5,  (WORLDLEVEL)7 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)5,  (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)0 }
};

const WmNavTable s_aaworldmapNavSnow =
{
    { (WORLDLEVEL)1,  (WORLDLEVEL)10, (WORLDLEVEL)10, (WORLDLEVEL)2 },
    { (WORLDLEVEL)7,  (WORLDLEVEL)0,  (WORLDLEVEL)4,  (WORLDLEVEL)2 },
    { (WORLDLEVEL)5,  (WORLDLEVEL)0,  (WORLDLEVEL)1,  (WORLDLEVEL)5 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)4,  (WORLDLEVEL)-1, (WORLDLEVEL)4 },
    { (WORLDLEVEL)3,  (WORLDLEVEL)10, (WORLDLEVEL)3,  (WORLDLEVEL)1 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)2,  (WORLDLEVEL)1,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)5,  (WORLDLEVEL)7,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)1,  (WORLDLEVEL)8,  (WORLDLEVEL)6 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)4,  (WORLDLEVEL)3,  (WORLDLEVEL)1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)4,  (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)0 }
};

const WmNavTable s_aaworldmapNavClockwerk =
{
    { (WORLDLEVEL)2,  (WORLDLEVEL)10, (WORLDLEVEL)2,  (WORLDLEVEL)4 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)3,  (WORLDLEVEL)0,  (WORLDLEVEL)-1, (WORLDLEVEL)3 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)0,  (WORLDLEVEL)2,  (WORLDLEVEL)4 },
    { (WORLDLEVEL)5,  (WORLDLEVEL)0,  (WORLDLEVEL)3,  (WORLDLEVEL)5 },
    { (WORLDLEVEL)6,  (WORLDLEVEL)4,  (WORLDLEVEL)4,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)8,  (WORLDLEVEL)5,  (WORLDLEVEL)8,  (WORLDLEVEL)-1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)3,  (WORLDLEVEL)-1, (WORLDLEVEL)6 },
    { (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)-1 },
    { (WORLDLEVEL)0,  (WORLDLEVEL)-1, (WORLDLEVEL)-1, (WORLDLEVEL)0 }
};
const WmNavTable* s_mpaaworldmapNav[GAMEWORLD_Max] =
{
    nullptr,
    &s_aaworldmapNavUnderwater,
    & s_aaworldmapNavMuggshot,
    & s_aaworldmapNavVoodoo,
    &s_aaworldmapNavSnow,
    &s_aaworldmapNavClockwerk
};

SNIP s_asnipWm[52] = {
    { 0x02, (OID)0x1CB, offsetof(WM, pasegOpen) },
    { 0x02, (OID)0x1CC, offsetof(WM, pasegClose) },

    { 0x02, (OID)0x1CD, offsetof(WM, awmw[0].pasegWarp) },
    { 0x02, (OID)0x1CE, offsetof(WM, awmw[1].pasegWarp) },
    { 0x02, (OID)0x1CF, offsetof(WM, awmw[2].pasegWarp) },
    { 0x02, (OID)0x1D0, offsetof(WM, awmw[3].pasegWarp) },
    { 0x02, (OID)0x1D1, offsetof(WM, awmw[4].pasegWarp) },
    { 0x02, (OID)0x1D2, offsetof(WM, awmw[5].pasegWarp) },
    { 0x02, (OID)0x1D3, offsetof(WM, awmw[6].pasegWarp) },
    { 0x02, (OID)0x1D4, offsetof(WM, awmw[7].pasegWarp) },
    { 0x02, (OID)0x1D5, offsetof(WM, awmw[8].pasegWarp) },
    { 0x02, (OID)0x1D6, offsetof(WM, awmw[10].pasegWarp) },

    { 0x02, (OID)0x1E2, offsetof(WM, awmw[0].paloSecondary) },
    { 0x02, (OID)0x1E3, offsetof(WM, awmw[1].paloSecondary) },
    { 0x02, (OID)0x1E4, offsetof(WM, awmw[2].paloSecondary) },
    { 0x02, (OID)0x1E5, offsetof(WM, awmw[3].paloSecondary) },
    { 0x02, (OID)0x1E6, offsetof(WM, awmw[4].paloSecondary) },
    { 0x02, (OID)0x1E7, offsetof(WM, awmw[5].paloSecondary) },
    { 0x02, (OID)0x1E8, offsetof(WM, awmw[6].paloSecondary) },
    { 0x02, (OID)0x1E9, offsetof(WM, awmw[7].paloSecondary) },
    { 0x02, (OID)0x1EA, offsetof(WM, awmw[8].paloSecondary) },
    { 0x02, (OID)0x1EC, offsetof(WM, awmw[10].paloSecondary) },
    { 0x02, (OID)0x1EB, offsetof(WM, awmw[9].paloSecondary) },

    { 0x02, (OID)0x1D7, offsetof(WM, awmw[0].paloPrimary) },
    { 0x02, (OID)0x1D8, offsetof(WM, awmw[1].paloPrimary) },
    { 0x02, (OID)0x1D9, offsetof(WM, awmw[2].paloPrimary) },
    { 0x02, (OID)0x1DA, offsetof(WM, awmw[3].paloPrimary) },
    { 0x02, (OID)0x1DB, offsetof(WM, awmw[4].paloPrimary) },
    { 0x02, (OID)0x1DC, offsetof(WM, awmw[5].paloPrimary) },
    { 0x02, (OID)0x1DD, offsetof(WM, awmw[6].paloPrimary) },
    { 0x02, (OID)0x1DE, offsetof(WM, awmw[7].paloPrimary) },
    { 0x02, (OID)0x1DF, offsetof(WM, awmw[8].paloPrimary) },
    { 0x02, (OID)0x1E1, offsetof(WM, awmw[10].paloPrimary) },
    { 0x02, (OID)0x1E0, offsetof(WM, awmw[9].paloPrimary) },

    { 0x02, (OID)0x1F8, offsetof(WM, awmw[0].ppntSecondary) },
    { 0x02, (OID)0x1F9, offsetof(WM, awmw[2].ppntSecondary) },
    { 0x02, (OID)0x1FA, offsetof(WM, awmw[3].ppntSecondary) },
    { 0x02, (OID)0x1FB, offsetof(WM, awmw[4].ppntSecondary) },
    { 0x02, (OID)0x1FC, offsetof(WM, awmw[5].ppntSecondary) },
    { 0x02, (OID)0x1FD, offsetof(WM, awmw[6].ppntSecondary) },
    { 0x02, (OID)0x1FE, offsetof(WM, awmw[7].ppntSecondary) },
    { 0x02, (OID)0x1FF, offsetof(WM, awmw[8].ppntSecondary) },

    { 0x02, (OID)0x1ED, offsetof(WM, awmw[0].ppntPrimary) },
    { 0x02, (OID)0x1EE, offsetof(WM, awmw[1].ppntPrimary) },
    { 0x02, (OID)0x1EF, offsetof(WM, awmw[2].ppntPrimary) },
    { 0x02, (OID)0x1F0, offsetof(WM, awmw[3].ppntPrimary) },
    { 0x02, (OID)0x1F1, offsetof(WM, awmw[4].ppntPrimary) },
    { 0x02, (OID)0x1F2, offsetof(WM, awmw[5].ppntPrimary) },
    { 0x02, (OID)0x1F3, offsetof(WM, awmw[6].ppntPrimary) },
    { 0x02, (OID)0x1F4, offsetof(WM, awmw[7].ppntPrimary) },
    { 0x02, (OID)0x1F5, offsetof(WM, awmw[8].ppntPrimary) },
    { 0x02, (OID)0x1F7, offsetof(WM, awmw[10].ppntPrimary) },
};

float g_rWmTaskMarkerScale = 0.3;
float g_sWmDotOuter = 24.0;
float g_sWmDotInner = 0.0;
const char* g_pchzWmTaskComplete = "k";
const char* g_pchzWmTaskIncomplete = "l";
