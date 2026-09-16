#include "bomb.h"
#include "expl.h"
#include "break.h"
#include "jt.h"
#include "bbmark.h"
#include "target.h"
#include "missile.h"

BOMB*NewBomb()
{
	return new BOMB{};
}

void InitBomb(BOMB* pbomb)
{
	InitSo(pbomb);

    pbomb->sDetonateRadiusJt = 200.0f;
    pbomb->tDetonate = FLT_MAX;
    pbomb->sDetonateRadius = 200.0f;
    pbomb->bdt = BDT_Impact;

    pbomb->lmMass = s_lmMassDefault;
    pbomb->sdvMax = 800.0f;
    pbomb->mMax = 5000.0f;
    pbomb->tDelay = 1.0f;

    pbomb->clqRadiusToSv = s_clqRadiusToSvBombDefault;
    pbomb->lmSv = s_lmSvBombDefault;

    pbomb->clqRadiusToSw = s_clqRadiusToSwBombDefault;
    pbomb->lmSw = s_lmSwBombDefault;

    pbomb->fReclaim = 1;
    pbomb->zpk = ZPK_Blunt;
    pbomb->fExplodeEffects = 1;
}

void SetBombSDetonateRadius(BOMB* pbomb, float sDetonateRadius)
{
    pbomb->sDetonateRadius = sDetonateRadius;
}

void* GetBombSDetonateRadius(BOMB* pbomb)
{
    return &pbomb->sDetonateRadius;
}

void SetBombSDetonateRadiusJt(BOMB* pbomb, float sDetonateRadiusJt)
{
    pbomb->sDetonateRadiusJt = sDetonateRadiusJt;
}

void* GetBombSDetonateRadiusJt(BOMB* pbomb)
{
    return &pbomb->sDetonateRadiusJt;
}

void SetBombBdt(BOMB* pbomb, BDT bdt)
{
    pbomb->bdt = bdt;
}

void* GetBombBdt(BOMB* pbomb)
{
    return &pbomb->bdt;
}

void SetBombBdm(BOMB* pbomb, BDM bdm)
{
    pbomb->bdm = bdm;
}

void* GetBombBdm(BOMB* pbomb)
{
    return &pbomb->bdm;
}

void SetBombFTouchDamage(BOMB* pbomb, int fTouchDamage)
{
    pbomb->fTouchDamage = fTouchDamage;
}

void* GetBombFTouchDamage(BOMB* pbomb)
{
    return &pbomb->fTouchDamage;
}

void SetBombSdvMax(BOMB* pbomb, float sdvMax)
{
    pbomb->sdvMax = sdvMax;
}

void* GetBombSdvMax(BOMB* pbomb)
{
    return &pbomb->sdvMax;
}

void SetBombMMax(BOMB* pbomb, float mMax)
{
    pbomb->mMax = mMax;
}

void* GetBombMMax(BOMB* pbomb)
{
    return &pbomb->mMax;
}

void SetBombLmMass(BOMB* pbomb, LM lmMass)
{
    pbomb->lmMass = lmMass;
}

void* GetBombLmMass(BOMB* pbomb)
{
    return &pbomb->lmMass;
}

void SetBombRExplodeScale(BOMB* pbomb, float rExplodeScale)
{
    pbomb->rExplodeScale = rExplodeScale;
}

void* GetBombRExplodeScale(BOMB* pbomb)
{
    return &pbomb->rExplodeScale;
}

void SetBombFExplodeEffects(BOMB* pbomb, int fExplodeEffects)
{
    pbomb->fExplodeEffects = fExplodeEffects;
}

void* GetBombFExplodeEffects(BOMB* pbomb)
{
    return &pbomb->fExplodeEffects;
}

void SetBombFReclaim(BOMB* pbomb, int fReclaim)
{
    pbomb->fReclaim = fReclaim;
}

void* GetBombFReclaim(BOMB* pbomb)
{
    return &pbomb->fReclaim;
}

void SetBombZpk(BOMB* pbomb, ZPK zpk)
{
    pbomb->zpk = zpk;
}

void* GetBombZpk(BOMB* pbomb)
{
    return &pbomb->zpk;
}

SFXID* PsfxEnsureBombSfxid(BOMB* pbomb, ENSK ensk)
{
    return &PsfxEnsureBomb(pbomb, ensk)->sfxid;
}

float* PsfxEnsureBombSStart(BOMB* pbomb, ENSK ensk)
{
    return &PsfxEnsureBomb(pbomb, ensk)->sStart;
}

float* PsfxEnsureBombSFull(BOMB* pbomb, ENSK ensk)
{
    return &PsfxEnsureBomb(pbomb, ensk)->sFull;
}

float* PsfxEnsureBombUVol(BOMB* pbomb, ENSK ensk)
{
    return &PsfxEnsureBomb(pbomb, ensk)->uVol;
}

void SetBombSvDetonate(BOMB* pbomb, float svDetonate)
{
    pbomb->svDetonate = svDetonate;
}

void* GetBombSvDetonate(BOMB* pbomb)
{
    return &pbomb->svDetonate;
}

void SetBombFCustomXps(BOMB* pbomb, int fCustomXps)
{
    pbomb->fCustomXps = fCustomXps;
}

void* GetBombFCustomXps(BOMB* pbomb)
{
    return &pbomb->fCustomXps;
}

void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pbomb, pbis);
    InferExpl(&pbomb->pexpl, pbomb);
}

void CloneBomb(BOMB* pbomb, BOMB* pbombBase)
{
    CloneSo(pbomb, pbombBase);

    pbomb->bdt = pbombBase->bdt;
    pbomb->tDetonate = pbombBase->tDetonate;
    pbomb->sDetonateRadius = pbombBase->sDetonateRadius;
    pbomb->sDetonateRadiusJt = pbombBase->sDetonateRadiusJt;
    pbomb->fTouchDamage = pbombBase->fTouchDamage;
    pbomb->tDelay = pbombBase->tDelay;
    pbomb->iDetonationProp = pbombBase->iDetonationProp;
    pbomb->sdvMax = pbombBase->sdvMax;
    pbomb->lmMass = pbombBase->lmMass;
    pbomb->mMax = pbombBase->mMax;
    pbomb->cpbrkp = pbombBase->cpbrkp;

    for (int i = 0; i < 16; ++i)
        pbomb->apbrkp[i] = pbombBase->apbrkp[i];

    pbomb->pexpl = pbombBase->pexpl;
    pbomb->rExplodeScale = pbombBase->rExplodeScale;
    pbomb->clqRadiusToSv = pbombBase->clqRadiusToSv;
    pbomb->lmSv = pbombBase->lmSv;
    pbomb->clqRadiusToSw = pbombBase->clqRadiusToSw;
    pbomb->lmSw = pbombBase->lmSw;
    pbomb->posDetonate = pbombBase->posDetonate;
    pbomb->normalDetonate = pbombBase->normalDetonate;
    pbomb->fExplodeEffects = pbombBase->fExplodeEffects;
    pbomb->fReclaim = pbombBase->fReclaim;
    pbomb->zpk = pbombBase->zpk;
    pbomb->psfxDet = pbombBase->psfxDet;

    if (pbombBase->psfxDet != nullptr)
        pbomb->psfxDet = std::make_shared<SFX>();
}


void PostBombLoad(BOMB* pbomb)
{
    PostAloLoad(pbomb);

    DLI dli;
    dli.m_pdl = &pbomb->dlChild;
    dli.m_ibDle = pbomb->dlChild.ibDle;
    dli.m_pdliNext = s_pdliFirst;

    LO* plo = static_cast<LO*>(pbomb->dlChild.paloFirst);
    dli.m_ppv = plo ? reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle) : nullptr;
    s_pdliFirst = &dli;

    while (plo != nullptr) 
    {
        if (FIsBasicDerivedFrom(plo, CID_BRKP) && pbomb->cpbrkp < 16U) {
            BRKP* pbrkp = static_cast<BRKP*>(plo);
            pbomb->apbrkp[pbomb->cpbrkp++] = pbrkp;
            SnipLo(plo);
        }

        plo = static_cast<LO*>(*dli.m_ppv);
        dli.m_ppv = plo ? reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle) : nullptr;
    }

    s_pdliFirst = dli.m_pdliNext;

    if (pbomb->fCustomXps != 0)
        pbomb->abombXp.resize(4);
}

void HandleBombMessage(BOMB* pbomb, MSGID msgid, void* pv)
{
    HandleAloMessage(pbomb, msgid, pv);

    if (msgid != MSGID_water_entered || pv == nullptr)
        return;

    const BombWaterEnteredMessage* pmsg = static_cast<const BombWaterEnteredMessage*>(pv);

    if (pmsg->pbomb == pbomb && pbomb->bdt == BDT_Impact)
        PrimeBomb(pbomb, 0.0f);
}

void UpdateBomb(BOMB* pbomb, float dt)
{
    const float speed = glm::length(pbomb->xf.v);

    if (g_clock.t >= pbomb->tDetonate || speed < pbomb->svDetonate) 
    {
        DetonateBomb(pbomb);
        return;
    }

    UpdateSo(pbomb, dt);

    if (pbomb->fCustomXps)
        UpdateBombCustomXps(pbomb);
}

void UpdateBombCustomXps(BOMB* pbomb)
{
    for (int ibombXp = 0; ibombXp < pbomb->cbombXp; ++ibombXp) {
        SO* psoOther = pbomb->abombXp[ibombXp].psoOther;

        if (psoOther == nullptr || psoOther->paloRoot == nullptr)
            continue;

        SO* psoRoot = static_cast<SO*>(psoOther->paloRoot);

        if (psoRoot->poxa != nullptr)
            UpdateSwPox(pbomb->psw, pbomb->poxa, psoRoot->poxa, 0, 64);
    }

    pbomb->cbombXp = 0;

    for (TARGET* ptarget = g_dlTarget.ptargetFirst; ptarget != nullptr; ptarget = ptarget->dleTarget.ptargetNext) 
    {
        if (ptarget->paloParent == nullptr)
            continue;

        if ((ptarget->grftak & 16U) == 0)
            continue;

        LO* ploOther = ptarget;

        while (ploOther != nullptr && (ploOther->pvtlo->grfcid & 2U) == 0)
            ploOther = ploOther->paloParent;

        if (ploOther == nullptr || !FIsLoInWorld(ploOther))
            continue;

        SO* psoOther = (SO*)ploOther;
        SO* psoRoot = (SO*)psoOther->paloRoot;

        glm::vec3 posTarget;
        LSG lsg{};

        GetXfmPos(ptarget, &posTarget);

        if (!ClsgClipEdgeToSphere(&posTarget, ptarget->sRadiusTarget, &pbomb->posWorldPrev, &pbomb->xf.posWorld, 1, &lsg))
            continue;

        if (psoRoot != nullptr && psoRoot->poxa != nullptr)
            UpdateSwPox(pbomb->psw, pbomb->poxa, psoRoot->poxa, 64, 0);

        if (pbomb->cbombXp >= static_cast<int>(pbomb->abombXp.size()))
            return;

        BOMBCUSTOMXP& bombXp = pbomb->abombXp[pbomb->cbombXp++];

        bombXp.pos = lsg.apos[0];
        bombXp.normal = lsg.anormal[0];
        bombXp.gDist = glm::dot(bombXp.pos, bombXp.normal);
        bombXp.psoOther = psoOther;
    }
}

void AddBombCustomXps(BOMB* pbomb, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    if (!pbomb->fCustomXps)
        return;

    for (int ibombXp = 0; ibombXp < pbomb->cbombXp; ++ibombXp) 
    {
        BOMBCUSTOMXP& bombXp = pbomb->abombXp[ibombXp];

        if (bombXp.psoOther != psoOther)
            continue;

        PxpAdd(pbomb, psoOther, &bombXp.pos, &bombXp.normal, bombXp.gDist, XPK_Internal, nullptr, nullptr, ppxpFirst);
        return;
    }
}

int GetBombSize()
{
	return sizeof(BOMB);
}

void AdjustBombNewXp(BOMB* pbomb, XP* pxp, int ixpd)
{
    if (pbomb->fDetonatePrepared)
        return;

    if (pbomb->bdt != BDT_Impact && pbomb->bdt != BDT_PropImpact)
        return;

    XPD& xpdImpact = pxp->axpd[1 - ixpd];
    SO* psoImpact = xpdImpact.psoLeaf;

    glm::vec3 posDetonate = pbomb->xf.posWorld;

    if (pbomb->bdt == BDT_Impact)
    {
        std::vector<SO*> apso = { psoImpact };
        LSG lsgHit = {};

        SO* psoHit = PsoHitTestLineObjects(0, &pbomb->posWorldPrev, &pbomb->xf.posWorld, apso, &lsgHit);

        if (psoHit != nullptr)
            posDetonate = lsgHit.apos[0];
    }
    else
    {
        PO* ppoCur = PpoCur();

        if (ppoCur == nullptr)
            return;

        if (!FFindLoParent(psoImpact, ppoCur))
            return;
    }

    glm::vec3 normalImpact = pxp->normal;

    if (ixpd != 0)
        normalImpact = -normalImpact;

    PrepareBombDetonation(pbomb, psoImpact, &posDetonate, &normalImpact);

    PrimeBomb(pbomb, 0.0f);
}
int FAbsorbBombWkr(BOMB* pbomb, WKR* pwkr)
{
    const int fAbsorbed = FAbsorbSoWkr(pbomb, pwkr);

    if (fAbsorbed && (pwkr->grftak & 16U) != 0) 
    {
        const float speed = glm::length(pwkr->v);
        const glm::vec3 normal = speed < 0.0001f ? g_normalZ : pwkr->v / speed;

        PrepareBombDetonation(pbomb, nullptr, nullptr, &normal);
        PrimeBomb(pbomb, 0.0f);
    }

    return fAbsorbed;
}

void PrepareBombDetonation(BOMB* pbomb, SO* psoImpact, const glm::vec3* ppos, const glm::vec3* pnormal)
{
    if (pbomb->fDetonatePrepared)
        return;

    pbomb->fDetonatePrepared = 1;

    pbomb->psoImpact = psoImpact;
    pbomb->posDetonate = ppos != nullptr ? *ppos : pbomb->xf.posWorld;
    pbomb->vDetonate = pbomb->xf.v;
    pbomb->wDetonate = pbomb->xf.w;

    switch (pbomb->bdm) 
    {
        case BDM_SurfaceNormal: 
        {
            const glm::vec3* pnormalDetonate = pnormal != nullptr ? pnormal : &g_normalZ;
            glm::mat3 matRotate;

            BuildRotateVectorsMatrix(&pbomb->xf.matWorld[2], pnormalDetonate, &matRotate);
            pbomb->matDetonate = matRotate * pbomb->xf.matWorld;

            ALO* paloTo = pbomb->paloParent;

            if (pbomb->palox && (pbomb->palox->grfalox & 8U) != 0)
                paloTo = pbomb->palox->foster.paloParent;

            ConvertAloMat(nullptr, paloTo, &pbomb->matDetonate, &pbomb->matDetonate);
            break;
        }

        case BDM_Velocity:
        BuildOrthonormalMatrixZ(pbomb->vDetonate, g_normalZ, pbomb->matDetonate);
        break;

        case BDM_LocalMatrix:
        pbomb->matDetonate = pbomb->xf.mat;
        break;
    }
}

void PrimeBomb(BOMB* pbomb, float dt)
{
    pbomb->tDetonate = g_clock.t + dt;
}

void ApplyBombThrow(BOMB* pbomb, PO* ppo)
{
    glm::vec3 normal;

    if (!FIsBasicDerivedFrom(pbomb, CID_ACCMISS))
        normal = ppo->xf.posWorld - pbomb->xf.posWorld;
    else
    {
        ACCMISS* paccmiss = reinterpret_cast<ACCMISS*>(pbomb);
        normal = paccmiss->vTarget;
    }

    normal.z = 0.0f;

    if (glm::length(normal) < 0.0001f)
        normal = g_normalX;
    else
        normal = glm::normalize(normal);

    glm::vec3 velocity = normal * VXY_BombThrowJt.x + g_normalZ * VXY_BombThrowJt.y;

    ApplySoConstraintWorld(ppo, &ppo->constrForce, &velocity, &velocity, nullptr);

    if (ppo->pvtalo->pfnSetAloVelocityVec != nullptr)
        ppo->pvtalo->pfnSetAloVelocityVec(ppo, &velocity);

    if (FIsBasicDerivedFrom(ppo, CID_JT))
        SetJtJts((JT*)ppo, JTS_Zap, (JTBS)43);
}

void DetonateBomb(BOMB* pbomb)
{
    PrepareBombDetonation(pbomb, nullptr, nullptr, nullptr);

    PO* ppo = PpoCur();

    std::vector <SO*> apso;

    if (!pbomb->fTouchDamage) {
        const float radiusQuery = glm::max(pbomb->sDetonateRadius, pbomb->sDetonateRadiusJt);
        IntersectSwBoundingSphere(pbomb->psw, nullptr, &pbomb->posDetonate, radiusQuery, nullptr, nullptr, apso);
    }
    else 
    {
        int cpso = 0;
        SO** apsoContacts = nullptr;

        GetSoContacts(pbomb, &cpso, &apsoContacts);

        if (cpso > 0)
            apso.assign(apsoContacts, apsoContacts + cpso);

		delete[] apsoContacts;

        if (pbomb->psoImpact != nullptr) {
            bool fImpactIncluded = false;

            for (SO* pso : apso) {
                if (pso == pbomb->psoImpact) {
                    fImpactIncluded = true;
                    break;
                }
            }

            if (!fImpactIncluded)
                apso.insert(apso.begin(), pbomb->psoImpact);
        }
    }

    for (SO* pso : apso) {
        if (pso->paloRoot == pbomb->paloRoot || pso->oid == OID__MERGED_STATICS)
            continue;

        if (FIsBasicDerivedFrom(pso, CID_BOMB)) 
        {
            BOMB* pbombOther = (BOMB*)pso;

            if (pbombOther->bdt != BDT_Timed) 
            {
                PrimeBomb(pbombOther, 0.1f);
                continue;
            }
        }

        if (!pbomb->fTouchDamage && pso->bspc.absp.size() != 0) {
            WKR wkrIntersect{};
            const float radius = pso->paloRoot == ppo ? pbomb->sDetonateRadiusJt : pbomb->sDetonateRadius;

            if (!CsbiIntersectSphereBsp(&pbomb->posDetonate, radius, pso->bspc.cbspFull, pso->bspc.absp.data(), nullptr, &pso->geomWorld, 1, reinterpret_cast<SBI*>(&wkrIntersect)))
                continue;
        }

        if (!FIsBasicDerivedFrom(pso, CID_PO)) 
        {
            if (pso->m < pbomb->mMax && pso->paloParent == nullptr) 
            {
                const glm::vec3 dpos = pso->xf.posWorld - pbomb->posDetonate;

                glm::vec3 direction = dpos;
                direction.z += 200.0f;

                const float directionLength = glm::length(direction);
                direction = directionLength < 0.0001f ? g_normalZ : direction / directionLength;

                const float mass = GLimitLm(&pbomb->lmMass, pso->m);
                const float radialVelocity = glm::dot(pso->xf.v, direction) / pbomb->sdvMax;

                const float udvRaw = s_clqRvToUdv.g0 + radialVelocity * (s_clqRvToUdv.g1 + radialVelocity * s_clqRvToUdv.g2);
                const float udv = GLimitLm(&g_lmZeroOne, udvRaw);

                const float surfaceDistance = glm::max(glm::length(dpos) - (pso->sRadiusSelf + pbomb->sRadiusAll), 0.0f);
                const float radiusRatio = surfaceDistance / pbomb->sDetonateRadius;

                const float uDistanceRaw = s_clqBombDist.g0 + radiusRatio * (s_clqBombDist.g1 + radiusRatio * s_clqBombDist.g2);
                const float uDistance = GLimitLm(&g_lmZeroOne, uDistanceRaw);

                const float sdv = pbomb->sdvMax * (1.0f / sqrtf(mass / pbomb->lmMass.gMin)) * udv * uDistance;

                glm::vec3 dv = direction * sdv;
                ApplySoConstraintLocal(pso, &pso->constrForce, &dv, &dv, nullptr);

                glm::vec3 velocity = pso->xf.v + dv;
                pso->pvtalo->pfnSetAloVelocityVec(pso, &velocity);

                if (sdv > 0.0001f)
                {
                    const float sw = sdv * 0.01f;
                    glm::vec3 dw(GRandInRange(-sw, sw), GRandInRange(-sw, sw), GRandInRange(-sw, sw));

                    ApplySoConstraintLocal(pso, &pso->constrTorque, &dw, &dw, nullptr);

                    glm::vec3 angularVelocity = pso->xf.w + dw;
                    pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &angularVelocity);
                }
            }

            WKR wkr{};
            wkr.pos = pbomb->posDetonate;
            wkr.v = pbomb->xf.v;
            wkr.grftak = 16;
            wkr.ploTarget = pso;
            wkr.ploSource = pbomb;

            PaloAbsorbWkr(&wkr, 0, nullptr);
        }
        else 
        {
            ZPR zpr{};

            InitZpr(&zpr, pbomb->zpk, pbomb);
            zpr.pfnzap = (PFNZAP)ApplyBombThrow;
            PO* ppoHit = static_cast<PO*>(pso);
            ppoHit->pvtpo->pfnFTakePoDamage(ppoHit, &zpr);
        }
    }

    ALO* paloTo = pbomb->paloParent;

    if (pbomb->palox && (pbomb->palox->grfalox & 4U) != 0)
        paloTo = pbomb->palox->foster.paloParent;

    glm::vec3 posDetonate;

    ConvertAloPos(nullptr, paloTo, &pbomb->posDetonate, &posDetonate);
    pbomb->pvtalo->pfnTranslateAloToPos(pbomb, &posDetonate);
    pbomb->pvtalo->pfnRotateAloToMat(pbomb, &pbomb->matDetonate);
    pbomb->pvtalo->pfnSetAloVelocityVec(pbomb, &pbomb->vDetonate);
    pbomb->pvtalo->pfnSetAloAngularVelocityVec(pbomb, &pbomb->wDetonate);

    if (pbomb->fExplodeEffects) {
        if (pbomb->pexpl == nullptr) 
            StandardSmokeCloud(&pbomb->xf.posWorld, pbomb->sDetonateRadius);
        else 
        {
            EXPLSO explso{};
            explso.rScale = pbomb->rExplodeScale;
            explso.sRadius = pbomb->sDetonateRadius;
            explso.grfexplso = 9;
            explso.paloReference = pbomb;

            pbomb->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pbomb->pexpl, &explso);
        }

        if (pbomb->psfxDet == nullptr)
            StartSound((SFXID)53, nullptr, nullptr, &pbomb->xf.posWorld, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        else
            StartSound(pbomb->psfxDet->sfxid, nullptr, nullptr, &pbomb->xf.posWorld, pbomb->psfxDet->sStart, pbomb->psfxDet->sFull, pbomb->psfxDet->uVol, 0.0f, 0.0f, nullptr, nullptr);
    }

    if (pbomb->cpbrkp > 0) {
        const float radius = pbomb->sDetonateRadius;

        const float svRaw = pbomb->clqRadiusToSv.g0 + radius * (pbomb->clqRadiusToSv.g1 + radius * pbomb->clqRadiusToSv.g2);
        const float swRaw = pbomb->clqRadiusToSw.g0 + radius * (pbomb->clqRadiusToSw.g1 + radius * pbomb->clqRadiusToSw.g2);

        const float sv = GLimitLm(&pbomb->lmSv, svRaw);
        const float sw = GLimitLm(&pbomb->lmSw, swRaw);

        for (int ipbrkp = 0; ipbrkp < pbomb->cpbrkp; ++ipbrkp) {
            BRKP* pbrkp = pbomb->apbrkp[ipbrkp];

            if (FIsLoInWorld(pbrkp))
                continue;

            pbrkp->pvtlo->pfnAddLo(pbrkp);
            pbrkp->pvtlo->pfnSetLoParent(pbrkp, nullptr);
            SetBrkpBrkps(pbrkp, BRKPS_Pending);
            pbrkp->pvtalo->pfnTranslateAloToPos(pbrkp, &pbomb->xf.posWorld);

            const glm::vec3 velocityLocal(GRandInRange(0.0f, sv * 2.5f), GRandInRange(-sv, sv), GRandInRange(-sv, sv));

            glm::vec3 velocityWorld = pbomb->xf.matWorld * velocityLocal;
            glm::vec3 angularVelocity(GRandInRange(-sw, sw), GRandInRange(-sw, sw), GRandInRange(-sw, sw));

            pbrkp->pvtalo->pfnSetAloVelocityVec(pbrkp, &velocityWorld);
            pbrkp->pvtalo->pfnSetAloAngularVelocityVec(pbrkp, &angularVelocity);
        }
    }

    pbomb->pvtlo->pfnRemoveLo(pbomb);
    HandleLoSpliceEvent(pbomb, 2, 0, nullptr);
    pbomb->fDetonatePrepared = 0;
    pbomb->tDetonate = FLT_MAX;
}

SFX* PsfxEnsureBomb(BOMB* pbomb, ENSK ensk)
{
    if (pbomb->psfxDet == nullptr)
        NewSfx(pbomb->psfxDet);

    return pbomb->psfxDet.get();
}

void DeleteBomb(BOMB *pbomb)
{
	delete pbomb;
}

LM s_lmMassDefault = {50.0, 5000};
CLQ s_clqRadiusToSvBombDefault = { 500, 1.8, 0.0 };
LM s_lmSvBombDefault = {600, 1400};
LM s_lmSwBombDefault = { 20, 50 };
CLQ s_clqRadiusToSwBombDefault = { 0.5, 0.2, 0.0 };
CLQ s_clqRvToUdv = {1.0, -1.0, 0.0};
CLQ s_clqBombDist = {2.0, -1.5, 0.0};
glm::vec3 VXY_BombThrowJt = {600, 800, 0.0};
