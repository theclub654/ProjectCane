#include "update.h"

void UpdateSw(SW* psw, float dt)
{
	UpdateSwObjects(psw, g_clock.dt);
	UpdateShaders(g_clock.dt);
	UpdateSwDarkening(psw, g_clock.dtReal);
}

void UpdateSwObjects(SW* psw, float dt)
{
    DLI dlBusyWalker;

    dlBusyWalker.m_pdl = &psw->dlBusy;
    dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;
    dlBusyWalker.m_pdliNext = s_pdliFirst;

    ALO* currentObject = psw->dlBusy.paloFirst;

    // Only set m_ppv if we actually have an object
    dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;

    // push
    s_pdliFirst = &dlBusyWalker;

    while (currentObject != nullptr)
    {
        UpdateAloHierarchy(currentObject, dt);

        currentObject = reinterpret_cast<ALO*>(*dlBusyWalker.m_ppv);

        dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;
    }

    // pop (CRITICAL to match original)
    s_pdliFirst = dlBusyWalker.m_pdliNext;
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

SMP g_smpDarkenIn{2.0, 0.0, 0.1};
SMP g_smpDarkenOut{1.0, 0.0, 0.1};