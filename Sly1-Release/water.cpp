#include "water.h"
#include "map2d.h"
#include "keyhole.h"
#include "zap.h"
#include "rip.h"
#include "puffer.h"
#include "jt.h"

WATER* NewWater()
{
	return new WATER{};
}

void InitWater(WATER* pwater)
{
	InitSo(pwater);
    pwater->fSplashDroplets = 1;
	pwater->fNoXpsSelf = 1;
	pwater->gBuoyancy = 1.0;
    pwater->fWaterWakeSound = 1;
	pwater->gViscosity = 1.0;
    SetSoConstraints(pwater, CT_Locked, nullptr, CT_Locked, nullptr);
    InitZpd(&pwater->zpd, pwater);
	pwater->zpd.zpk = ZPK_Water;
}

void* GetWaterFSplash(WATER* pwater)
{
    return &pwater->fSplash;
}

void SetWaterFSplash(WATER* pwater, int fSplash)
{
    pwater->fSplash = fSplash;
}

void* GetWaterFZap(WATER* pwater)
{
    return &pwater->fZap;
}

void SetWaterFZap(WATER* pwater, int fZap)
{
    pwater->fZap = fZap;
}

void* GetWaterVCurrent(WATER* pwater)
{
    return &pwater->vCurrent;
}

void SetWaterVCurrent(WATER* pwater, glm::vec3 vCurrent)
{
    pwater->vCurrent = vCurrent;
}

void* GetWaterDzThrow(WATER* pwater)
{
    return &pwater->zpd.dzThrow;
}

void SetWaterDzThrow(WATER* pwater, float dzThrow)
{
    pwater->zpd.dzThrow = dzThrow;
}

void* GetWaterFThrowCut(WATER* pwater)
{
    return &pwater->zpd.fThrowCut;
}

void SetWaterFThrowCut(WATER* pwater, int fThrowCut)
{
    pwater->zpd.fThrowCut = fThrowCut;
}

void* GetWaterFWaterWakeSound(WATER* pwater)
{
    return &pwater->fWaterWakeSound;
}

void SetWaterFWaterWakeSound(WATER* pwater, int fWaterWakeSound)
{
    pwater->fWaterWakeSound = fWaterWakeSound;
}

void* GetWaterFSplashDroplets(WATER* pwater)
{
    return &pwater->fSplashDroplets;
}

void SetWaterFSplashDroplets(WATER* pwater, int fSplashDroplets)
{
    pwater->fSplashDroplets = fSplashDroplets;
}

void ResetWaterThrowCount(WATER* pwater)
{
    pwater->zpd.cploThrow = 0;
}

int GetWaterSize()
{
	return sizeof(WATER);
}

void CloneWater(WATER* pwater, WATER* pwaterBase)
{
	CloneSo(pwater, pwaterBase);

	pwater->pxaTargets = pwaterBase->pxaTargets;
	pwater->mrg = pwaterBase->mrg;
	pwater->vCurrent = pwaterBase->vCurrent;
	pwater->fSplash = pwaterBase->fSplash;
	pwater->fZap = pwaterBase->fZap;
	pwater->zpd = pwaterBase->zpd;
}

void PostWaterLoad(WATER* pwater)
{
    DLI dli;

    dli.m_pdl = &pwater->dlChild;
    dli.m_ppv = reinterpret_cast<void**>(dli.m_pdl);
    dli.m_ibDle = pwater->dlChild.ibDle;
    dli.m_pdliNext = s_pdliFirst;

    s_pdliFirst = &dli;

    PostAloLoad(pwater);
    PostZpdLoad(&pwater->zpd);

    pwater->mrg.apalo.clear();
    pwater->mrg.apalo.reserve(32);

    if (pwater->globset.pwrbgFirst != nullptr) 
    {
        if (pwater->pvtso->pfnUpdateSoBounds != nullptr)
            pwater->pvtso->pfnUpdateSoBounds(pwater);
    }

    s_pdliFirst = dli.m_pdliNext;
}

void CalculateWaterCurrent(WATER* pwater, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pw)
{
    glm::vec3 posLocal;
    glm::vec3 vLocal = pwater->dvCurrent;
    glm::vec3 wLocal = g_vecZero;

    ConvertAloVec(nullptr, pwater, ppos, &posLocal);

    posLocal.z = 0.0f;

    CalculateAloTransformAdjust(pwater, nullptr, &posLocal, nullptr, &vLocal, &wLocal);

    if (pwater->globset.pwrbgFirst != nullptr) {
        glm::vec3 vWarp;
        WarpWrTransform(pwater->globset.pwrbgFirst->pwr, 50.0f, ppos, nullptr, nullptr, nullptr, &vWarp);
        vLocal += vWarp;
    }

    if (pv != nullptr)
        *pv = vLocal;

    if (pw != nullptr)
        *pw = wLocal;
}

void UpdateSwXaList(SW* psw, std::shared_ptr<XA>* ppxa)
{
    std::shared_ptr<XA> pxaFree;

    while (*ppxa != nullptr)
    {
        std::shared_ptr <XA> pxa = *ppxa;
        std::shared_ptr <XA> pxaNext = pxa->pxaNextTarget;
        SO* psoTarget = pxa->psoTarget;

        bool fWasInList = psoTarget->fUpdateXaList1 != 0;
        bool fIsInList = psoTarget->fUpdateXaList2 != 0;

        if (fWasInList != fIsInList)
        {
            if (!fWasInList)
                AddSoXa(psoTarget, pxa.get());
            else
                RemoveSoXa(psoTarget, pxa.get());
        }

        if (!fIsInList)
        {
            *ppxa = pxaNext;
            pxa->pxaNextTarget = pxaFree;
            pxaFree = pxa;
        }
        else
        {
            ppxa = &pxa->pxaNextTarget;
        }

        psoTarget->fUpdateXaList1 = 0;
        psoTarget->fUpdateXaList2 = 0;
    }

    FreeSwXaList(psw, pxaFree);
}

void UpdateWater(WATER* pwater, float dt)
{
    UpdateSo(pwater, dt);

    if (pwater->bspc.absp.size() == 0)
        return;

    SW* psw = pwater->psw;
    bool fWaterStateChanged = false;

    for (XA* pxa = pwater->pxaTargets.get(); pxa != nullptr; pxa = pxa->pxaNextTarget.get())
        pxa->psoTarget->fUpdateXaList1 = 1;

    OX* pox = nullptr;
    SO* psoRoot = (SO*)pwater->paloRoot;

    if (pwater->paloRoot != nullptr && psoRoot->poxa != nullptr)
        pox = psoRoot->poxa->pox;

    while (pox != nullptr)
    {
        SO* psoOther = pox->psoOther;
        pox = pox->poxNext;

        if (psoOther->gBuoyancy < 0.0001f)
            continue;

        if (psoOther->fNoXpsAll || psoOther->fLockedSelf)
            continue;

        float uSubmerged = UGetWaterSubmerged(pwater, psoOther, nullptr, nullptr);

        if (uSubmerged == 0.0f)
            continue;

        psoOther->fUpdateXaList2 = 1;

        if (pwater->globset.pwrbgFirst != nullptr)
            ResolveAlo(psoOther);

        if (psoOther->fUpdateXaList1)
            continue;

        std::shared_ptr <XA> pxa = std::make_shared<XA>();
        pxa->psoSource = pwater;
        pxa->psoTarget = psoOther;
        pxa->pxaNextTarget = pwater->pxaTargets;
        pwater->pxaTargets = pxa;

        if (!pwater->fSplash || psoOther->xf.v.z >= -100.0f)
            continue;

        glm::vec3 dposWarp(0.0f);
        WRBG* pwrbg = pwater->globset.pwrbgFirst.get();

        if (pwrbg != nullptr)
        {
            glm::vec3 posWarp;
            WarpWrTransform(pwrbg->pwr, 50.0f, &psoOther->xf.posWorld, nullptr, &posWarp, nullptr, nullptr);
            dposWarp = posWarp - psoOther->xf.posWorld;
        }

        glm::vec3 apos[2];
        apos[0] = psoOther->posWorldPrev - dposWarp;
        apos[1] = psoOther->xf.posWorld - dposWarp;

        // The release uses a stable feet offset for Sly. His animated bounds
        // change with pose, so posMin would move the splash below the surface.
        const float dzBottom =
            psoOther == static_cast<SO*>(g_pjt)
            ? -75.0f
            : psoOther->posMin.z - psoOther->xf.posWorld.z;
        apos[0].z += dzBottom;
        apos[1].z += dzBottom;

        LSG alsg[16];
        int clsg = ClsgClipEdgeToBsp(pwater->bspc.absp.data(), &apos[0], &apos[1], nullptr, 16, alsg);

        if (clsg <= 0 || alsg[0].data.bsp.apsurf[0] == nullptr)
            continue;

        float vz = psoOther->xf.v.z;
        float uVolAtSource = std::min(-vz, 6.0f) * (1.0f / 6.0f);
        float sSplash = ((psoOther->posMax.x - psoOther->posMin.x) + (psoOther->posMax.y - psoOther->posMin.y)) * 0.5f;

        if (vz < 0.0f)
        {
            StartSound((SFXID)34, nullptr, psoOther, nullptr, 10.0f, 1.0f, uVolAtSource, 0.0f, 0.0f, nullptr, nullptr);
            alsg[0].apos[0] += dposWarp;
            StockSplashBig(alsg[0].apos, sSplash * uVolAtSource, pwater);
        }
    }

    for (XA* pxa = pwater->pxaTargets.get(); pxa != nullptr; pxa = pxa->pxaNextTarget.get())
    {
        SO* psoTarget = pxa->psoTarget;

        bool fWasInWater = psoTarget->fUpdateXaList1 != 0;
        bool fIsInWater = psoTarget->fUpdateXaList2 != 0;

        if (fWasInWater == fIsInWater)
            continue;

        fWaterStateChanged = true;
        psoTarget->fWater = fIsInWater;

        struct
        {
            WATER* pwater;
            SO* pso;
        } waterMessage = { pwater, psoTarget };

        MSGID msgid = fWasInWater ? MSGID_water_left : MSGID_water_entered;

        psoTarget->pvtlo->pfnSendLoMessage(psoTarget, msgid, &waterMessage);
    }

    UpdateSwXaList(psw, &pwater->pxaTargets);

    if (fWaterStateChanged)
        UpdateWaterMergeGroup(pwater);
}

void AddWaterExternalAccelerations(WATER* pwater, XA* pxa, float dt)
{
    pxa->psoTarget->pvtso->pfnAddSoWaterAcceleration(pxa->psoTarget, pwater, dt);
}

void HandleWaterMessage(WATER* pwater, MSGID msgid, void* pv)
{
    bool fWaterGroupChanged = false;

    if (msgid == MSGID_removed)
    {
        if (pv == pwater)
        {
            for (std::shared_ptr<XA> pxa = pwater->pxaTargets; pxa != nullptr; pxa = pxa->pxaNextTarget)
                RemoveSoXa(pxa->psoTarget, pxa.get());

            FreeSwXaList(pwater->psw, pwater->pxaTargets);
            pwater->pxaTargets.reset();

            fWaterGroupChanged = true;
        }
        else
        {
            SO* psoMessage = static_cast<SO*>(pv);
            std::shared_ptr <XA>* ppxa = &pwater->pxaTargets;

            while (*ppxa != nullptr && (*ppxa)->psoTarget != psoMessage)
                ppxa = &(*ppxa)->pxaNextTarget;

            if (*ppxa != nullptr)
            {
                std::shared_ptr<XA> pxaRemoved = *ppxa;
                SO* psoRemoved = pxaRemoved->psoTarget;

                *ppxa = pxaRemoved->pxaNextTarget;
                pxaRemoved->pxaNextTarget.reset();

                RemoveSoXa(psoRemoved, pxaRemoved.get());

                MSGWATER msgwater;
                msgwater.pwater = pwater;
                msgwater.pso = psoRemoved;

                psoRemoved->pvtlo->pfnSendLoMessage(psoRemoved, MSGID_water_left, &msgwater);

                FreeSwXaList(pwater->psw, pxaRemoved);
                fWaterGroupChanged = true;
            }
        }
    }

    if (fWaterGroupChanged)
        UpdateWaterMergeGroup(pwater);

    HandleAloMessage(pwater, msgid, pv);
}

void UpdateWaterMergeGroup(WATER* pwater)
{
    if (pwater->paloRoot != pwater)
        return;

    RemoveSwMergeGroup(pwater->psw, &pwater->mrg);

    pwater->mrg.apalo.clear();
    pwater->mrg.apalo.push_back(pwater);

    for (XA* pxa = pwater->pxaTargets.get(); pxa != nullptr; pxa = pxa->pxaNextTarget.get())
        pwater->mrg.apalo.push_back(pxa->psoTarget);

    AddSwMergeGroup(pwater->psw, &pwater->mrg);
}

float UGetWaterSubmerged(WATER* pwater, SO* pso, glm::vec3* pposSurface, glm::vec3* pnormalSurface)
{
    glm::vec3 apos[2];

    if (pwater->globset.pwrbgFirst == nullptr) {
        apos[0] = pso->xf.posWorld;
        apos[1] = pso->xf.posWorld;
    }
    else
    {
        WarpWrTransform(pwater->globset.pwrbgFirst->pwr, 50.0f, &pso->xf.posWorld, nullptr, apos, nullptr, nullptr);

        glm::vec3 posWarped = apos[0];
        glm::vec3 posMirrored = pso->xf.posWorld * 2.0f - posWarped;

        apos[0] = posMirrored;
        apos[1] = posMirrored;
    }

    const float zWorld = pso->xf.posWorld.z;

    apos[0].z += pso->posMin.z - zWorld;
    apos[1].z += pso->posMax.z - zWorld;

    LSG alsg[16];

    int clsg = ClsgClipEdgeToBsp(pwater->bspc.absp.data(), &apos[0], &apos[1], nullptr, 16, alsg);

    if (clsg == 0) {
        if (pnormalSurface != nullptr) {
            *pnormalSurface = g_normalZ;
        }

        return 0.0f;
    }

    const LSG& lsg = alsg[0];

    if (pposSurface != nullptr) {
        *pposSurface = lsg.apos[1];
    }

    if (pnormalSurface != nullptr) {
        SURF* psurf = lsg.data.bsp.apsurf[1];

        if (psurf != nullptr) {
            *pnormalSurface = psurf->normal;
        }
        else {
            *pnormalSurface = g_normalZ;
        }
    }

    return lsg.au[1] - lsg.au[0];
}

void UpdateWaterBounds(WATER* pwater)
{
	UpdateSoBounds(pwater);

	WRBG* pwrbg = pwater->globset.pwrbgFirst.get();
	if (!pwrbg || !pwrbg->pwr)
		return;

	glm::vec3 dpos{};
	GetWrBounds(pwrbg->pwr, &dpos);

	float radiusAdd = glm::length(dpos);

	pwater->posMin -= dpos;
	pwater->posMax += dpos;

	pwater->sRadiusPrune += radiusAdd;
	pwater->sRadiusAll   += radiusAdd;
}

int FInflictWaterZap(WATER* pwater, XP* pxp, ZPR* pzpr)
{
    InflictZpdZap(&pwater->zpd, pxp, pzpr);
    return 1;
}

void* PzpdEnsureWater(WATER* pwater, ENSK ensk)
{
    return &pwater->zpd;
}

void WakeSoWater(SO* pso, WATER* pwater, float dt, glm::vec3* pvCurrent, glm::vec3* pposSurface, float dtWakeMin, float gScale)
{
    if (!pwater->fSplash)
        return;

    const float relativeSpeed  = glm::length(*pvCurrent - pso->xf.v);
    const bool useWakeInterval = dt <= GRandInRange(0.0f, 3.0f);

    PUFFER* ppuffer = nullptr;

    if ((pso->pvtlo->grfcid & 8) != 0)
        ppuffer = (PUFFER*)pso;

    if (relativeSpeed <= 10.0f && useWakeInterval)
    {
        if (ppuffer != nullptr)
            StopSound(ppuffer->pambWake, 0);

        return;
    }

    const float wakeStrength = glm::clamp((relativeSpeed - 10.0f) * 0.0012658228f, 0.0f, 1.0f);

    if (ppuffer != nullptr && pwater->fWaterWakeSound)
    {
        const float wakeVolume = wakeStrength * 0.9f + 0.1f;

        if (ppuffer->pambWake == nullptr)
        {
            const int soundIndex = NRandInRange(0, 3);
            StartSound((SFXID)asfxidWade[soundIndex], &ppuffer->pambWake, ppuffer, nullptr, 10.0f, 1.0f, wakeVolume, 0.0f, 0.0f, nullptr, nullptr);
        }
        else
            SetPambVol(ppuffer->pambWake, wakeVolume);
    }

    if (useWakeInterval)
    {
        const float rippleInterval = 1.0f - wakeStrength * (1.0f - dtWakeMin);
        bool emitRipple = false;

        if (ppuffer == nullptr)
        {
            const int previousInterval = static_cast<int>(g_clock.t / rippleInterval);
            const int nextInterval = static_cast<int>((g_clock.t + dt) / rippleInterval);
            emitRipple = previousInterval != nextInterval;
        }
        else
            emitRipple = g_clock.t - ppuffer->tWakeRipple > rippleInterval;

        if (!emitRipple)
            return;
    }

    RIP* pripple = PripNewRipg(RIPT_Ripple, nullptr);

    if (pripple == nullptr)
        return;

    pripple->pvtrip->pfnInitRip(pripple, pposSurface, gScale, nullptr);

    if (ppuffer != nullptr)
        ppuffer->tWakeRipple = g_clock.t;

    if (!pwater->fSplashDroplets || relativeSpeed <= 500.0f)
        return;

    EMITRIP emitrip{};

    emitrip.ript = RIPT_Droplet;
    emitrip.riptTrail = RIPT_Nil;
    emitrip.clqScale.g0 = 1.0f;
    emitrip.lmGScale.gMin = 8.0f;
    emitrip.lmGScale.gMax = 12.0f;
    emitrip.psoTouch = pwater;

    EMITV emitv{};

    // The original scales wake-droplet launch speed from the object's
    // velocity relative to the water current.  Leaving this range at the
    // zero-initialized default makes the droplets bunch up at the surface.
    emitv.lmSv.gMin = relativeSpeed * 0.5f;
    emitv.lmSv.gMax = relativeSpeed * 0.75f;
    emitv.rSvz = 1.0f;
    emitv.dv = pso->psw->dvGravity;
    emitv.normalCurl = g_normalZ;
    emitv.lmTilt.gMin = 0.4f;
    emitv.lmTilt.gMax = 1.0471976f;
    emitv.uRandomRad = 1.0f;
    emitv.dtLifetime = 5.0f;
    emitv.clqAlpha.g0 = 1.0f;
    emitv.clqAlpha.g2 = -1.0f;

    glm::vec3 dropletPosition = *pposSurface;
    dropletPosition.z += 30.0f;

    const int dropletCount = NRandInRange(4, 6);
    EmitRipsSphere(&dropletPosition, &g_normalZ, dropletCount, &emitrip, &emitv, nullptr, nullptr);
}

void DeleteWater(WATER *pwater)
{
	delete pwater;
}

CLQ s_clqDampV = {0.0, 3.0, 0.0};
CLQ s_clqDampW = {0.0, 0.75, 0.0};
int asfxidWade[4] =
{
    0x52,
    0x53,
    0x54,
    0x55
};
