#include "blip.h"

// Retail leaves this debug switch disabled by default.  When enabled it adds
// an extra identity instance in addition to the emitter-group members.
int g_fDrawEmitterGroupSelf = 0;

BLIPG* NewBlipg()
{
	return new BLIPG{};
}

void InitSwBlipgDl(SW* psw)
{
	InitDl(&psw->dlBlipg, offsetof(BLIPG, dleBlipg));
}

void InitSwBlipgFreeDl(SW* psw)
{
	InitDl(&psw->dlBlipgFree, offsetof(BLIPG, dleBlipg));
}

void InitBlipg(BLIPG* pblipg)
{
    AppendDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
    InitAlo(pblipg);
    InitDl(&pblipg->dlBlip, offsetof(BLIP, dle));
    pblipg->sMRD = 1e+10;
    pblipg->mrds = 2;
    pblipg->fNoFreeze = 1;
}

int GetBlipgSize()
{
	return sizeof(BLIPG);
}

void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase)
{
    CloneAlo(pblipg, pblipgBase);

    pblipg->pemitter = pblipgBase->pemitter;
    pblipg->clqScale = pblipgBase->clqScale;
    pblipg->clqAlpha = pblipgBase->clqAlpha;
    pblipg->clqTexture = pblipgBase->clqTexture;
    pblipg->clqColor = pblipgBase->clqColor;
    pblipg->blipmk = pblipgBase->blipmk;
    pblipg->blipgm = pblipgBase->blipgm;
    pblipg->pshd = pblipgBase->pshd;
    pblipg->crgba = pblipgBase->crgba;
    for (int i = 0; i < 32; ++i)
        pblipg->argba[i] = pblipgBase->argba[i];
    pblipg->fColorRanges = pblipgBase->fColorRanges;
    pblipg->cqwTexture = pblipgBase->cqwTexture;
    for (int i = 0; i < 32; ++i)
        pblipg->aiframeTexture[i] = pblipgBase->aiframeTexture[i];
    pblipg->blipok = pblipgBase->blipok;
    pblipg->rSFlying = pblipgBase->rSFlying;
    pblipg->cblipe = pblipgBase->cblipe;
    pblipg->dlBlip = pblipgBase->dlBlip;
    pblipg->dleBlipg = pblipgBase->dleBlipg;
}

void OnBlipgAdd(BLIPG* pblipg)
{
    RemoveDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
    AppendDlEntry(&pblipg->psw->dlBlipg, pblipg);
    OnAloAdd(pblipg);
}

void OnBlipgRemove(BLIPG* pblipg)
{
    while (pblipg->dlBlip.ploFirst != nullptr)
    {
        BLIP* pblip = pblipg->dlBlip.pblipFirst;
        RemoveBlip(pblip);
    }

    RemoveDlEntry(&pblipg->psw->dlBlipg, pblipg);
    AppendDlEntry(&g_dlBlipgPending, pblipg);
    pblipg->pchzName = const_cast<char*>("Unused BLIP Group");

    OnAloRemove(pblipg);
}

void SetBlipgShader(BLIPG *pblipg, OID oid)
{
    pblipg->pshd = PshdFindShader(oid);

    if (pblipg->pshd == nullptr) {
        pblipg->pshd = &g_ashd[0];
    }

    // EmitBlips selects a color with NRandInRange(0, crgba - 1), so a
    // shader-backed group must still expose one selectable color when the
    // emitter has no explicit color table.
    if (pblipg->crgba <= 0)
    {
        pblipg->argba[0] = pblipg->pshd->rgba;
        pblipg->crgba = 1;
    }

    PropagateBlipgShader(pblipg);
}

void PropagateBlipgShader(BLIPG* pblipg)
{
    if (pblipg == nullptr)
        return;

    pblipg->cqwTexture = 1;
    pblipg->aiframeTexture[0] = 0;

    if (pblipg->pshd == nullptr || pblipg->pshd->atex.empty())
        return;

    const TEX& tex = pblipg->pshd->atex[0];
    const int cframe = std::min<int>(tex.cibmp, 32);
    if (cframe <= 0)
        return;

    // The PS2 builds a power-of-two packet table and limits it to 32 slots.
    int cframePadded = 1;
    while (cframePadded < cframe)
        cframePadded <<= 1;
    while (cframePadded > 32)
        cframePadded >>= 1;

    pblipg->cqwTexture = cframePadded;

    // This matches the packet insertion order in the original function. The
    // first frames are duplicated until the power-of-two table is filled.
    const int cduplicate = cframePadded - cframe;
    int itexture = 0;
    for (int iframe = 0; iframe < cframe && itexture < cframePadded; ++iframe)
    {
        pblipg->aiframeTexture[itexture++] = iframe;
        if (iframe < cduplicate && itexture < cframePadded)
            pblipg->aiframeTexture[itexture++] = iframe;
    }

    // Defensive completion for malformed shader data. Valid BRX data reaches
    // cframePadded through the loop above.
    while (itexture < cframePadded)
        pblipg->aiframeTexture[itexture++] = cframe - 1;
}

void PropagateBlipgShaders(GRFZON grfzon)
{
    if (g_psw == nullptr || grfzon == 0)
        return;

    for (BLIPG* pblipg = g_psw->dlBlipg.pblipgFirst;
         pblipg != nullptr;
         pblipg = pblipg->dleBlipg.pblipgNext)
    {
        SHD* pshd = pblipg->pshd;
        if (pshd == nullptr)
            continue;

        const GRFZON grfzonShader = pshd->grfzon;
        if ((grfzonShader & 0x10000000u) != 0 ||
            (grfzon & grfzonShader) == grfzon)
        {
            PropagateBlipgShader(pblipg);
        }
    }

}

void SetBlipgEmitb(BLIPG* pblipg, EMITB* pemitb)
{
    if (pblipg == nullptr || pemitb == nullptr)
        return;

    EMITBLIP& emitblip = pemitb->emitp.emitblip;
    EMITV& emitv = pemitb->emitv;

    pblipg->pchzName = pemitb->pchzName;
    pblipg->blipok = emitblip.blipok;
    pblipg->rSFlying = emitblip.rSFlying;
    pblipg->crgba = std::min(emitblip.crgba,
                             static_cast<int>(emitblip.argba.size()));
    pblipg->crgba = std::min(pblipg->crgba, 32);
    pblipg->crgba = std::max(pblipg->crgba, 0);

    for (int i = 0; i < pblipg->crgba; ++i)
        pblipg->argba[i] = emitblip.argba[i];

    pblipg->fColorRanges = emitblip.fColorRanges;

    SetBlipgShader(pblipg, emitblip.oidShader);

    std::memset(&pblipg->clqTexture, 0, sizeof(pblipg->clqTexture));

    if (pblipg->cqwTexture > 1)
    {
        if (emitblip.dtShaderLoop == 0.0f)
        {
            if (emitblip.fShaderSpan == 0)
                pblipg->clqTexture = emitblip.clqTexture;
            else
                pblipg->clqTexture.g1 = static_cast<float>(pblipg->cqwTexture) / emitv.dtLifetime;
        }
        else
            pblipg->clqTexture.g1 = static_cast<float>(pblipg->cqwTexture) / emitblip.dtShaderLoop;
    }

    PrescaleClq(&emitv.clqAlpha, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqAlpha);
    PrescaleClq(&emitblip.clqScale, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqScale);
    PrescaleClq(&emitblip.clqColor, 1.0f / emitv.dtLifetime, 0.0f, &pblipg->clqColor);

    pblipg->blipmk = emitblip.blipmk;

    if (pblipg->blipmk == BLIPMK_Accel)
    {
        const glm::vec3& dv = emitv.dv;
        const bool zeroDv =
            std::abs(dv.x) < 0.0001f &&
            std::abs(dv.y) < 0.0001f &&
            std::abs(dv.z) < 0.0001f;

        if (zeroDv && emitv.rvDamping == 0.0f && emitv.swCurl == 0.0f)
        {
            pblipg->blipmk = BLIPMK_Constant;
        }
    }

    std::memset(&pblipg->blipgm, 0, sizeof(pblipg->blipgm));

    if (pblipg->blipmk == BLIPMK_Accel)
    {
        SetEmitdvEmitb(reinterpret_cast<EMITDV*>(&pblipg->blipgm), pemitb);
    }
}

void UpdateBlipg(BLIPG* pblipg, float dt)
{
    UpdateAlo(pblipg, dt);
    ResolveAlo(pblipg);

    GRFZON objZoneMask = pblipg->pshd->grfzon;

    bool inCameraZone = (objZoneMask & 0x10000000u) || ((g_pcm->grfzon & objZoneMask) == g_pcm->grfzon);

    if (!inCameraZone)
    {
        pblipg->pvtlo->pfnRemoveLo(pblipg);
    }
}

void SubscribeBlipgObject(BLIPG* pblipg, EMITTER* ploTarget)
{
    SubscribeLoObject(pblipg, ploTarget);

    if (FIsBasicDerivedFrom(ploTarget, CID_EMITTER))
    {
        EMITTER *pemitter = (EMITTER*)ploTarget;

        pblipg->pemitter = pemitter;
        pblipg->sMRD     = pemitter->sMRD;

        if (!FIsDlEmpty(&pemitter->dlGroup))
        {
            pblipg->viss = 0;
            pblipg->mrds = 0;
        }
    }
}

void ProjectBlipgTransformAccel(BLIPG* pblipg, float dt)
{
    if (pblipg == nullptr)
        return;

    glm::mat4 mat4Dv(1.0f);
    CalculateEmitdvMatrix(&pblipg->blipgm.emitdv, dt, &mat4Dv);

    glm::vec3 posMin(FLT_MAX);
    glm::vec3 posMax(-FLT_MAX);

    for (BLIP* pblip = pblipg->dlBlip.pblipFirst; pblip != nullptr;)
    {
        BLIP* const pblipNext = pblip->dle.pblipNext;
        const int iblipfNext = (pblip->iblipfLatest + 1) & 1;
        BLIPF& blipfOld = pblip->ablipf[pblip->iblipfLatest];
        BLIPF& blipfNew = pblip->ablipf[iblipfNext];
        int cblipeNew = 0;

        for (int iblipe = 0; iblipe < pblip->cblipe; ++iblipe)
        {
            const BLIPP& posOld = blipfOld.ablipp[iblipe];
            const BLIPV& vOld = blipfOld.ablipv[iblipe];

            if (vOld.tDestroy <= g_clock.t)
                continue;

            glm::vec3 velocity = glm::vec3(mat4Dv * glm::vec4(vOld.x, vOld.y, vOld.z, 1.0f));
            glm::vec3 pos(posOld.x, posOld.y, posOld.z);
            pos += velocity * dt;

            BLIPP& posNew = blipfNew.ablipp[cblipeNew];
            BLIPV& vNew = blipfNew.ablipv[cblipeNew];
            BLIPX& xNew = blipfNew.ablipx[cblipeNew];

            posNew = { pos.x, pos.y, pos.z, posOld.tCreated };
            vNew = { velocity.x, velocity.y, velocity.z, vOld.tDestroy };
            xNew = blipfOld.ablipx[iblipe];

            if (pblipg->blipok == BLIPOK_Rolling)
                xNew.radRoll = RadNormalize(xNew.radRoll + xNew.swRoll * dt);

            posMin = glm::min(posMin, pos);
            posMax = glm::max(posMax, pos);
            ++cblipeNew;
        }

        const int cblipeRemoved = pblip->cblipe - cblipeNew;
        pblip->cblipe = cblipeNew;
        pblipg->cblipe -= cblipeRemoved;

        if (cblipeNew == 0)
            RemoveBlip(pblip);
        else
            pblip->iblipfLatest = iblipfNext;

        pblip = pblipNext;
    }

    if (pblipg->cblipe <= 0)
    {
        pblipg->cblipe = 0;
        pblipg->pvtlo->pfnRemoveLo(pblipg);
        return;
    }

    glm::vec3 posCenter = (posMin + posMax) * 0.5f;
    pblipg->pvtalo->pfnTranslateAloToPos(pblipg, &posCenter);
    const float radius = glm::length(posCenter - posMax);
    pblipg->sRadiusRenderSelf = radius;
    pblipg->sRadiusRenderAll = radius;

}

void ProjectBlipgTransform(BLIPG* pblipg, float dt, int fForce)
{
    if (pblipg == nullptr)
        return;

    if (pblipg->blipmk == BLIPMK_Accel)
    {
        ProjectBlipgTransformAccel(pblipg, dt);
        return;
    }

    glm::vec3 posMin(FLT_MAX);
    glm::vec3 posMax(-FLT_MAX);

    for (BLIP* pblip = pblipg->dlBlip.pblipFirst; pblip != nullptr;)
    {
        BLIP* const pblipNext = pblip->dle.pblipNext;
        const int iblipfNext = (pblip->iblipfLatest + 1) & 1;
        BLIPF& blipfOld = pblip->ablipf[pblip->iblipfLatest];
        BLIPF& blipfNew = pblip->ablipf[iblipfNext];
        int cblipeNew = 0;

        for (int iblipe = 0; iblipe < pblip->cblipe; ++iblipe)
        {
            const BLIPP& posOld = blipfOld.ablipp[iblipe];
            const BLIPV& vOld = blipfOld.ablipv[iblipe];

            if (vOld.tDestroy <= g_clock.t)
                continue;

            BLIPP& posNew = blipfNew.ablipp[cblipeNew];
            BLIPV& vNew = blipfNew.ablipv[cblipeNew];
            BLIPX& xNew = blipfNew.ablipx[cblipeNew];

            glm::vec3 pos(posOld.x, posOld.y, posOld.z);
            glm::vec3 velocity(vOld.x, vOld.y, vOld.z);

            if (pblipg->blipmk == BLIPMK_Constant)
                pos += velocity * dt;
            else if (pblipg->blipmk == BLIPMK_Spline && pblip->pblipsp != nullptr)
            {
                const float dtRemaining = vOld.tDestroy - g_clock.t;
                const float dtStep = std::min(dtRemaining, dt);
                BLIPSX& spline = pblip->pblipsp->ablipsx[iblipe];
                glm::vec3 posResult;
                glm::vec3 velocityResult;

                EvaluateBezierPos(dtRemaining, dtStep, 1.0f, &pos, &velocity, &spline.posFinal, &spline.vFinal, &posResult, &velocityResult, nullptr);
                pos = posResult;
                velocity = velocityResult;

                if (cblipeNew != iblipe)
                    pblip->pblipsp->ablipsx[cblipeNew] = spline;
            }

            posNew = { pos.x, pos.y, pos.z, posOld.tCreated };
            vNew = { velocity.x, velocity.y, velocity.z, vOld.tDestroy };
            xNew = blipfOld.ablipx[iblipe];

            if (pblipg->blipok == BLIPOK_Rolling)
                xNew.radRoll = RadNormalize(xNew.radRoll + xNew.swRoll * dt);

            posMin = glm::min(posMin, pos);
            posMax = glm::max(posMax, pos);
            ++cblipeNew;
        }

        const int cblipeRemoved = pblip->cblipe - cblipeNew;
        pblip->cblipe = cblipeNew;
        pblipg->cblipe -= cblipeRemoved;

        if (cblipeNew == 0)
            RemoveBlip(pblip);
        else
            pblip->iblipfLatest = iblipfNext;

        pblip = pblipNext;
    }

    if (pblipg->cblipe <= 0)
    {
        pblipg->cblipe = 0;
        pblipg->pvtlo->pfnRemoveLo(pblipg);
        return;
    }

    glm::vec3 posCenter = (posMin + posMax) * 0.5f;
    pblipg->pvtalo->pfnTranslateAloToPos(pblipg, &posCenter);
    const float radius = glm::length(posCenter - posMax);
    pblipg->sRadiusRenderSelf = radius;
    pblipg->sRadiusRenderAll = radius;
}

void EmitBlips(EMITB* pemitb, EMITG* pemitg, int cblipeRequested, const glm::vec3* apos, const glm::vec3* av, const float* atCreated, const float* atDestroy, const glm::vec3* aposFinal, const glm::vec3* avFinal)
{
    if (!pemitb || cblipeRequested <= 0 || !apos || !av || !atCreated || !atDestroy)
        return;

    auto& emitblip = pemitb->emitp.emitblip;

    BLIPG* pblipg = nullptr;

    if (pemitg && pemitg->ppblipg)
        pblipg = *pemitg->ppblipg;

    if (!pblipg)
    {
        /*if (!g_psw->slotheapBlip.pslotFree)
            return;

        if (emitblip.blipmk == BLIPMK_Spline && !g_psw->slotheapBlipsp.pslotFree)
            return;*/

        pblipg = PblipgNew(g_psw);

        if (!pblipg)
            return;

        if (pemitg && pemitg->ppblipg)
            *pemitg->ppblipg = pblipg;

        SetBlipgEmitb(pblipg, pemitb);

        if (pemitg && pemitg->ploSubscribe)
            pblipg->pvtlo->pfnSubscribeLoObject(pblipg, pemitg->ploSubscribe);

    }

    BLIP* pblip = pblipg->dlBlip.pblipFirst;
    int iblipeRequested = 0;

    while (iblipeRequested < cblipeRequested)
    {
        if (!pblip)
        {
            pblip = PblipNew(pblipg);

            if (!pblip)
                return;
        }

        while (pblip->cblipe < 44 && iblipeRequested < cblipeRequested)
        {
            const int iblip = pblip->cblipe;

            BLIPF& blipf = pblip->ablipf[0];
            BLIPP& blipp = blipf.ablipp[iblip];
            BLIPV& blipv = blipf.ablipv[iblip];
            BLIPX& blipx = blipf.ablipx[iblip];

            blipp.x = apos[iblipeRequested].x;
            blipp.y = apos[iblipeRequested].y;
            blipp.z = apos[iblipeRequested].z;
            blipp.tCreated = atCreated[iblipeRequested];

            blipv.x = av[iblipeRequested].x;
            blipv.y = av[iblipeRequested].y;
            blipv.z = av[iblipeRequested].z;
            blipv.tDestroy = atDestroy[iblipeRequested];

            blipx = {};

            blipx.irgba = NRandInRange(0, pblipg->crgba - 1);

            if (pblipg->fColorRanges)
                blipx.irgba &= ~1u;

            if (emitblip.fRandomFrame)
                blipx.itex0 = NRandInRange(0, pblipg->cqwTexture - 1);

            if (pblipg->blipok == BLIPOK_Rolling)
            {
                blipx.swRoll = GRandInRange(emitblip.lmSw.gMin, emitblip.lmSw.gMax);

                if (emitblip.bliprk == BLIPRK_Mirror)
                {
                    const glm::vec3 cameraAxis = glm::vec3(g_pcm->mat[1]);

                    if (glm::dot(cameraAxis, av[iblipeRequested]) > 0.0f)
                        blipx.swRoll = -blipx.swRoll;
                }

                if (emitblip.fRandomRoll)
                    blipx.radRoll = GRandInRange(-glm::pi<float>(), glm::pi<float>());
            }

            if (pblipg->blipmk == BLIPMK_Spline && pblip->pblipsp && aposFinal && avFinal)
            {
                BLIPSX& blipsx = pblip->pblipsp->ablipsx[iblip];
                blipsx.posFinal = aposFinal[iblipeRequested];
                blipsx.vFinal = avFinal[iblipeRequested];
            }

            // Initialize both simulation frames with identical data.
            pblip->ablipf[1].ablipp[iblip] = blipp;
            pblip->ablipf[1].ablipv[iblip] = blipv;
            pblip->ablipf[1].ablipx[iblip] = blipx;

            ++pblip->cblipe;
            ++pblipg->cblipe;
            ++iblipeRequested;
        }

        pblip = pblip->dle.pblipNext;
    }

}

void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro)
{
    if (pblipg == nullptr)
        return;

    const float alpha = pro != nullptr ? pro->uAlpha : 1.0f;

    auto submit = [pblipg, alpha](const glm::mat4& model, float instanceAlpha = 1.0f)
    {
        RPL rpl{};
        rpl.rp = RP_Blip;
        rpl.z = 0.0f;
        rpl.pblipg = pblipg;
        rpl.ro.model = model;
        rpl.ro.uAlpha = alpha * instanceAlpha;
        SubmitRpl(&rpl);
    };

    auto submitVisible = [pblipg, pcm, &submit](EMITTER* pgroup, const glm::mat4& model)
    {
        if (pcm == nullptr || pgroup == nullptr)
            return "invalid";

        // DrawBlipg applies the group object's explicit zone restriction only
        // for VISS_Zone (2), then performs its sphere/MRD rejection.
        if (pgroup->viss == 2 &&
            (pgroup->grfzon & pcm->grfzon) != pcm->grfzon)
            return "zone";

        // DrawBlipg culls around the BLIPG's local particle center transformed
        // by this group member, not around the member translation alone.
        const glm::vec3 posWorld = glm::vec3(model * glm::vec4(pblipg->xf.pos, 1.0f));
        const glm::vec3 dpos = posWorld - pcm->pos;
        if (!SphereInFrustum(pcm->frustum, posWorld, pblipg->sRadiusRenderAll))
            return "frustum";

        float mrdAlpha = 1.0f;
        if (!FInsideCmMrd(pcm, dpos, pblipg->sRadiusRenderAll,
                          pgroup->sMRD, mrdAlpha))
            return "mrd";

        submit(model, mrdAlpha);
        return "submitted";
    };

    // The original renderer reused one BLIP particle buffer for every member
    // of an emitter group.  Each member supplied its own world/merge matrix.
    EMITTER* const pemitter = pblipg->pemitter;
    if (pemitter != nullptr && !FIsDlEmpty(&pemitter->dlGroup))
    {
        // This is a disabled-by-default debug path in retail.
        if (g_fDrawEmitterGroupSelf != 0)
            submit(glm::mat4(1.0f));

        for (EMITTER* pgroup = pemitter->dlGroup.pemitterFirst;
             pgroup != nullptr;
             pgroup = pgroup->dleGroup.pemitterNext)
        {
            const int inWorld = FIsLoInWorld(pgroup);
            if (!inWorld)
                continue;

            glm::vec3 posWorld = pgroup->xf.posWorld;
            glm::mat3 matWorld(1.0f);

            // Retail DrawBlipg does not use the group member's own rotation.
            // It combines the member's world position with its parent's world
            // rotation. This matters for fixed BLIPs such as the swamp torch
            // flames: applying the member's authored rotation turns their
            // locally rising particles sideways.
            //
            // Merged members use the merge anchor for both components, which
            // is the separate apmrg[0] path in the original renderer.
            MRG* const pmrg = pgroup->cpmrg > 0 ? pgroup->apmrg[0] : nullptr;
            if (pmrg != nullptr && !pmrg->apalo.empty() && pmrg->apalo[0] != nullptr)
            {
                ALO* const paloMerge = pmrg->apalo[0];
                posWorld = paloMerge->xf.posWorld;
                matWorld = paloMerge->xf.matWorld;
            }
            else if (pgroup->paloParent != nullptr)
            {
                matWorld = pgroup->paloParent->xf.matWorld;
            }

            glm::mat4 model(1.0f);
            LoadMatrixFromPosRot(&posWorld, &matWorld, &model);

            submitVisible(pgroup, model);
        }

        // A nonempty emitter group is represented only by the accepted member
        // transforms (plus the optional debug self).  If all members are
        // culled, original DrawBlipg returns instead of drawing at the origin.
        return;
    }

    // Non-group BLIP positions have already been converted to world space by
    // ConvertEmitoPosVec.  The original DrawBlipg explicitly supplied the
    // identity matrix in this case; using the BLIPG ALO matrix here applies
    // its projected bounds-center translation a second time.
    submit(glm::mat4(1.0f));
}

void DeleteBlipg(BLIPG* pblipg)
{
	delete pblipg;
}

BLIP* PblipNew(BLIPG* pblipg)
{
    if (!pblipg || !pblipg->psw)
        return nullptr;

    SW* psw = pblipg->psw;

    auto blipOwner = std::make_unique <BLIP>();
    BLIP* pblip = blipOwner.get();

    pblip->pblipg = pblipg;
    pblip->pblipsp = nullptr;

    if (pblipg->blipmk == BLIPMK_Spline)
    {
        auto blipspOwner = std::make_unique <BLIPSP>();

        pblip->pblipsp = blipspOwner.get();
        psw->blipspOwners.push_back(std::move(blipspOwner));
    }

    psw->blipOwners.push_back(std::move(blipOwner));
    AppendDlEntry(&pblipg->dlBlip, pblip);

    return pblip;
}

BLIPG* PblipgNew(SW* psw)
{
    BLIPG* pblipg = psw->dlBlipgFree.pblipgFirst;

	if (pblipg != nullptr)
	{
		pblipg->pemitter = nullptr;
		pblipg->sMRD = 1.0e10f;
		pblipg->viss = 2;
		pblipg->mrds = 2;
		pblipg->cblipe = 0;
		pblipg->pvtlo->pfnAddLo(pblipg);
		pblipg->pchzName = const_cast<char*>("BLIP Group");
	}

	return pblipg;
}

void RemoveBlip(BLIP* pblip)
{
    RemoveDlEntry(&pblip->pblipg->dlBlip, pblip);
    AppendDlEntry(&g_dlBlipPending, pblip);
}

DL g_dlBlipgPending;
DL g_dlBlipPending;
