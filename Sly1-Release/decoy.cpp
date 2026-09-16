#include "decoy.h"
#include "jt.h"
#include "act.h"

DECOY* NewDecoy()
{
	return new DECOY{};
}

void* GetDecoyCHitsRemaining(DECOY* pdecoy)
{
    return &pdecoy->cHitsRemaining;
}

void SetDecoyCHitsRemaining(DECOY* pdecoy, int cHitsRemaining)
{
    pdecoy->cHitsRemaining = cHitsRemaining;
}

void* GetDecoyPexplDestroy(DECOY* pdecoy)
{
    return &pdecoy->pexplDestroy;
}

void SetDecoyPexplDestroy(DECOY* pdecoy, EXPL* pexplDestroy)
{
    pdecoy->pexplDestroy = pexplDestroy;
}

void CloneDecoy(DECOY* pdecoy, DECOY* pdecoyBase)
{
    CloneSo(pdecoy, pdecoyBase);

    pdecoy->cHitsRemaining = pdecoyBase->cHitsRemaining;
    pdecoy->pxp = pdecoyBase->pxp;
    pdecoy->pexplDestroy = pdecoyBase->pexplDestroy;
}

void PostDecoyLoad(DECOY* pdecoy)
{
    PostAloLoad(pdecoy);
    SnipAloObjects(pdecoy, 1, &snipDecoyPostLoad);
    pdecoy->pvtlo->pfnRemoveLo(pdecoy);
}

void DamageJtDecoy(DECOY* pdecoy, JT* pjt, int fForceDestroy)
{
    if (fForceDestroy == 0)
    {
        DECOYDAMAGE damage = { pdecoy, pjt };

        pdecoy->pvtlo->pfnHandleLoMessage(pdecoy, (MSGID)23, &damage);

        --pdecoy->cHitsRemaining;

        if (pdecoy->cHitsRemaining > 0)
            return;
    }

    ExplodeExpl(pdecoy->pexplDestroy);
    StartSound((SFXID)103, nullptr, nullptr, &pdecoy->xf.posWorld, 4000.0f, 750.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    HandleLoSpliceEvent(pdecoy, 2, 0, nullptr);
    pdecoy->pvtlo->pfnRemoveLo(pdecoy);
}

void UpdateDecoy(DECOY* pdecoy, float dt)
{
    UpdateSo(pdecoy, dt);

    if (g_pjt != nullptr)
    {
        const glm::vec3 dposJt = pdecoy->xf.posWorld - g_pjt->xf.posWorld;

        if (glm::length(dposJt) > 2000.0f)
            DamageJtDecoy(pdecoy, g_pjt, 1);
        else if (pdecoy->paloParent == nullptr && pdecoy->pxp != nullptr)
        {
            XP* pxp = pdecoy->pxp;
            const int ixpdOther = pdecoy->paloRoot == pxp->axpd[0].psoRoot;
            JT* pjtOther = reinterpret_cast<JT*>(pxp->axpd[ixpdOther].psoLeaf);

            DamageJtDecoy(pdecoy, pjtOther, 0);
        }
    }
    else if (pdecoy->paloParent == nullptr && pdecoy->pxp != nullptr)
    {
        XP* pxp = pdecoy->pxp;
        const int ixpdOther = pdecoy->paloRoot == pxp->axpd[0].psoRoot;
        JT* pjtOther = reinterpret_cast<JT*>(pxp->axpd[ixpdOther].psoLeaf);

        DamageJtDecoy(pdecoy, pjtOther, 0);
    }

    FreeSwXpList(pdecoy->psw, pdecoy->pxp);
    pdecoy->pxp = nullptr;
}

void PresetDecoyAccel(DECOY* pdecoy, float dt)
{
    PresetSoAccel(pdecoy, dt);

    if (pdecoy->paloParent != nullptr)
        return;

    glm::mat3 matUpright(1.0f);

    TiltMatUpright(&pdecoy->xf.mat, nullptr, &matUpright);
    AccelSoTowardMatSpring(pdecoy, &matUpright, pdecoy->pclqRotSpring.get(), &g_normalY, pdecoy->pclqRotDamping.get(), dt);
}

void AdjustDecoyXps(DECOY* pdecoy)
{
    AdjustSoZapXps(pdecoy, &pdecoy->pxp);
}

bool FAbsorbDecoyWkr(DECOY* pdecoy, WKR* pwkr)
{
    const bool fAbsorbed = FAbsorbSoWkr(pdecoy, pwkr);

    if (!fAbsorbed || (pwkr->grfic & 32) == 0)
        return fAbsorbed;

    LO* ploSource = pwkr->ploSource;
    SO* psoSource = reinterpret_cast<SO*>(ploSource);
    SO* psoSourceRoot;

    if ((psoSource->pvtlo->grfcid & 1) == 0)
        psoSourceRoot = (SO*)psoSource->paloParent->paloRoot;
    else
        psoSourceRoot = (SO*)psoSource->paloRoot;

    DamageJtDecoy(pdecoy, reinterpret_cast<JT*>(ploSource), psoSourceRoot == g_pjt);

    FreeSwXpList(pdecoy->psw, pdecoy->pxp);
    pdecoy->pxp = nullptr;

    return true;
}

void UpdateDecoyXfWorldHierarchy(DECOY* pdecoy)
{
    if (pdecoy->pactScale == nullptr)
    {
        UpdateSoXfWorldHierarchy(pdecoy);
        return;
    }

    UpdateAloXfWorldHierarchy(pdecoy);

    glm::mat3 matScale(1.0f);
    pdecoy->pactScale->pvtact->pfnGetActScale(pdecoy->pactScale, &matScale);

    const glm::mat3& matWorld = pdecoy->xf.matWorld;

    glm::mat3 matGeomWorld;

    matGeomWorld[0].x = matWorld[0].x * matScale[0].x + matWorld[1].x * matScale[0].y + matWorld[2].x * matScale[0].z;
    matGeomWorld[0].y = matWorld[0].y * matScale[0].x + matWorld[1].y * matScale[0].y + matWorld[2].y * matScale[0].z;
    matGeomWorld[0].z = matWorld[0].z * matScale[0].x + matWorld[1].z * matScale[0].y + matWorld[2].z * matScale[0].z;

    matGeomWorld[1].x = matWorld[0].x * matScale[1].x + matWorld[1].x * matScale[1].y + matWorld[2].x * matScale[1].z;
    matGeomWorld[1].y = matWorld[0].y * matScale[1].x + matWorld[1].y * matScale[1].y + matWorld[2].y * matScale[1].z;
    matGeomWorld[1].z = matWorld[0].z * matScale[1].x + matWorld[1].z * matScale[1].y + matWorld[2].z * matScale[1].z;

    matGeomWorld[2].x = matWorld[0].x * matScale[2].x + matWorld[1].x * matScale[2].y + matWorld[2].x * matScale[2].z;
    matGeomWorld[2].y = matWorld[0].y * matScale[2].x + matWorld[1].y * matScale[2].y + matWorld[2].y * matScale[2].z;
    matGeomWorld[2].z = matWorld[0].z * matScale[2].x + matWorld[1].z * matScale[2].y + matWorld[2].z * matScale[2].z;

    UpdateGeomWorld(&pdecoy->geomLocal, &pdecoy->geomWorld, pdecoy->xf.posWorld, matGeomWorld);
}

void HandleDecoyMessage(DECOY* pdecoy, int msgid, void* pv)
{
    HandleAloMessage(pdecoy, (MSGID)msgid, pv);

    if (msgid != 10)
        return;

    SO* psoWater = *reinterpret_cast<SO**>(pv);

    PxpAdd(pdecoy, psoWater, &pdecoy->xf.posWorld, &g_normalX, 0.0f, XPK_Internal, nullptr, nullptr, &pdecoy->pxp);
}

void DeleteDecoy(DECOY* pdecoy)
{
    delete pdecoy;
}
