#include "update.h"
#include "so.h"
#include "bbmark.h"
#include "po.h"
#include "hide.h"
#include "cm.h"

void UpdateSw(SW* psw, float dt)
{
    UpdateSwRealClock(psw, g_clock.dtReal);

    if (g_clock.dt != 0.0f)
    {
        UpdateSwObjects(psw, g_clock.dt);
        UpdateSwPosWorldPrev(psw);

        RecalcSwXpAll(psw, 0);
        SolveSw(psw, g_clock.dt, 1);

        // Update pivots for busy physical objects.
        {
            DLI dli{};
            dli.m_pdl = &psw->dlBusySo;
            dli.m_ibDle = psw->dlBusySo.ibDle;
            dli.m_pdliNext = s_pdliFirst;

            s_pdliFirst = &dli;

            SO* pso = reinterpret_cast<SO*>(psw->dlBusySo.pvFirst);

            while (pso)
            {
                dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pso) + dli.m_ibDle);

                if (pso->pvtso->pfnUpdateSoPivots)
                    pso->pvtso->pfnUpdateSoPivots(pso, g_clock.dt);

                pso = static_cast<SO*>(*dli.m_ppv);
            }

            s_pdliFirst = dli.m_pdliNext;
        }

        RecalcSwXpAll(psw, 1);
        SolveSw(psw, g_clock.dt, 6);

        // Resolve active ALOs.
        DLI dliBusy{};
        dliBusy.m_pdl = &psw->dlBusy;
        dliBusy.m_ibDle = psw->dlBusy.ibDle;
        dliBusy.m_pdliNext = s_pdliFirst;

        s_pdliFirst = &dliBusy;

        ALO* palo = psw->dlBusy.paloFirst;

        while (palo)
        {
            dliBusy.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(palo) + dliBusy.m_ibDle);

            if (palo->cframeStatic < 2)
            {
                if (FIsAloStatic(palo))
                    ++palo->cframeStatic;
                else
                    ResolveAlo(palo);
            }

            palo = static_cast<ALO*>(*dliBusy.m_ppv);
        }

        // Impact impulses belong to the world's contact graph, not only to
        // objects currently present in dlBusySo.  Static level barriers (for
        // example lethal falls and the car crushers in A Rocky Start) can be
        // the side that owns pfnNotifySoImpact, so drain every physical root.
        DLI dliRoot{};
        dliRoot.m_pdl = &psw->dlRoot;
        dliRoot.m_ibDle = psw->dlRoot.ibDle;
        dliRoot.m_pdliNext = s_pdliFirst;

        s_pdliFirst = &dliRoot;

        SO* psoRoot = psw->dlRoot.psoFirst;

        while (psoRoot)
        {
            dliRoot.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(psoRoot) + dliRoot.m_ibDle);

            psoRoot->pvtso->pfnUpdateSoImpacts(psoRoot);
            psoRoot = static_cast<SO*>(*dliRoot.m_ppv);
        }

        s_pdliFirst = dliRoot.m_pdliNext;

        // Touching events are generated only for active objects that requested
        // them through OnTouch/OnUntouch bindings.
        dliBusy.m_pdl = &psw->dlBusySo;
        dliBusy.m_ibDle = psw->dlBusySo.ibDle;

        SO* pso = psw->dlBusySo.psoFirst;

        while (pso)
        {
            dliBusy.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pso) + dliBusy.m_ibDle);

            if (pso->fGenSpliceTouchEvents)
                GenerateSoSpliceTouchingEvents(pso);

            pso = static_cast<SO*>(*dliBusy.m_ppv);
        }

        s_pdliFirst = dliBusy.m_pdliNext;

        ProcessSwSpliceScheduledCallbacks(psw, g_clock.dt);
        ProcessSwCallbacks(psw);
        UpdateShaders(g_clock.dt);

	}

    // Retail continues evaluating the active camera policy while gameplay is
    // paused. UpdateCptn keeps game-driven movement on the zero game dt, but
    // applies manual orbit input with dtReal so the camera can still rotate
    // behind the pause menu.
    UpdateCmLast(g_pcm, 1, g_clock.dt);

	// 989SND continues servicing voices and adaptive music while the gameplay
	// clock is paused. The pause UI only ducks MVGK groups to 60 percent.
	UpdateSounds();

	UpdateSwHud(psw);

    // Remove animations queued for deferred removal.
    {
        DLI dliPending{};
        dliPending.m_pdl = &psw->dlAsegaPending;
        dliPending.m_ibDle = psw->dlAsegaPending.ibDle;
        dliPending.m_pdliNext = s_pdliFirst;

        s_pdliFirst = &dliPending;

        ASEGA* pasega = psw->dlAsegaPending.pasegaFirst;

        while (pasega)
        {
            dliPending.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pasega) + dliPending.m_ibDle);
            RemoveAsega(pasega);
            pasega = static_cast<ASEGA*>(*dliPending.m_ppv);
        }

        s_pdliFirst = dliPending.m_pdliNext;
    }

    UpdateSwDarkening(psw, g_clock.dtReal);
}

void UpdateSwRealClock(SW* psw, float dt)
{
    if (!psw || dt == 0.0f)
        return;

    DLI dli{};
    dli.m_pdliNext = s_pdliFirst;
    s_pdliFirst = &dli;

    // Update real-clock model hierarchies.
    dli.m_pdl = &psw->dlMRDRealClock;
    dli.m_ibDle = psw->dlMRDRealClock.ibDle;

    ALO* palo = psw->dlMRDRealClock.paloFirst;

    while (palo)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(palo) + dli.m_ibDle);

        UpdateAloHierarchy(palo, dt);

        palo = static_cast<ALO*>(*dli.m_ppv);
    }

    // Update animations that use the real clock.
    dli.m_pdl = &psw->dlAsegaRealClock;
    dli.m_ibDle = psw->dlAsegaRealClock.ibDle;

    ASEGA* pasega = psw->dlAsegaRealClock.pasegaFirst;

    while (pasega)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pasega) + dli.m_ibDle);

        UpdateAsega(pasega, dt);

        pasega = static_cast<ASEGA*>(*dli.m_ppv);
    }

    // Update faders that use the real clock.
    dli.m_pdl = &psw->dlRealClockFader;
    dli.m_ibDle = psw->dlRealClockFader.ibDle;

    FADER* pfader = psw->dlRealClockFader.pfaderFirst;

    while (pfader)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pfader) + dli.m_ibDle);

        UpdateFader(pfader, dt);

        pfader = static_cast<FADER*>(*dli.m_ppv);
    }

    // Project transforms after animations and faders have updated.
    dli.m_pdl = &psw->dlMRDRealClock;
    dli.m_ibDle = psw->dlMRDRealClock.ibDle;

    palo = psw->dlMRDRealClock.paloFirst;

    while (palo)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(palo) + dli.m_ibDle);

        if (palo->pvtalo->pfnProjectAloTransform)
            palo->pvtalo->pfnProjectAloTransform(palo, dt, 0);

        palo = static_cast<ALO*>(*dli.m_ppv);
    }

    s_pdliFirst = dli.m_pdliNext;

    ProcessSwCallbacks(psw);
}

void UpdateSwObjects(SW* psw, float dt)
{
    if (!psw)
        return;

    PO* ppo = PpoCur();

    if (ppo && ppo->pvtpo->pfnUpdatePoActive)
    {
		JOY* pjoy = (g_grfjoyt & 2) ? &g_joy : &g_joyZero;
        ppo->pvtpo->pfnUpdatePoActive(ppo, pjoy, dt);
    }

    UpdateSwBusyList(psw);

    DLI dli{};
    dli.m_pdliNext = s_pdliFirst;
    s_pdliFirst = &dli;

    // Update active ALO hierarchies.
    dli.m_pdl = &psw->dlBusy;
    dli.m_ibDle = psw->dlBusy.ibDle;

    ALO* palo = psw->dlBusy.paloFirst;

    while (palo)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(palo) + dli.m_ibDle);

        UpdateAloHierarchy(palo, dt);
        palo = static_cast<ALO*>(*dli.m_ppv);
    }

    // Update active animations.
    dli.m_pdl = &psw->dlAsega;
    dli.m_ibDle = psw->dlAsega.ibDle;

    ASEGA* pasega = reinterpret_cast<ASEGA*>(psw->dlAsega.pvFirst);

    while (pasega)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pasega) + dli.m_ibDle);

        UpdateAsega(pasega, dt);
        pasega = static_cast<ASEGA*>(*dli.m_ppv);
    }

    // Update active faders.
    dli.m_pdl = &psw->dlFader;
    dli.m_ibDle = psw->dlFader.ibDle;

    FADER* pfader = reinterpret_cast<FADER*>(psw->dlFader.pvFirst);

    while (pfader)
    {
        dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pfader) + dli.m_ibDle);

        UpdateFader(pfader, dt);
        pfader = static_cast<FADER*>(*dli.m_ppv);
    }

    s_pdliFirst = dli.m_pdliNext;

	FireSwTimedExplodeStyles(psw);
	ProcessSwCallbacks(psw);
}

void UpdateSwDarkening(SW* psw, float dt)
{
    SMP *psmp;

    float gCur = psw->rDarken;
    float gTarget = psw->rDarkenSmooth;

    if (gTarget < gCur)
		psmp = &g_smpDarkenIn;
    else 
	{
        if (gTarget <= gCur)
            return;

        psmp = &g_smpDarkenOut;
    }

    psw->rDarken = GSmooth(gCur, gTarget, dt, psmp, nullptr);
}

void UpdateSwPosWorldPrev(SW* psw)
{
    for (SO* psoBusy = psw->dlBusySo.psoFirst; psoBusy != nullptr; psoBusy = psoBusy->dleBusySo.psoNext)
    {
        for (SO* psoPhys = psoBusy->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
        {
            if (!psoPhys->fCenterXp)
                psoPhys->pvtso->pfnUpdateSoPosWorldPrev(psoPhys);

            psoPhys->fCenterXp = 0;
        }
    }
}

SMP g_smpDarkenIn{2.0, 0.0, 0.1};
SMP g_smpDarkenOut{1.0, 0.0, 0.1};
