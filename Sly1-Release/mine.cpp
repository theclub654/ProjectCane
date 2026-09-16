#include "mine.h"
#include "jt.h"

MINE* NewMine()
{
    return new MINE{};
}

void CloneMine(MINE *pmine, MINE *pmineBase)
{
    CloneBomb(pmine, pmineBase);
}

void DeployJtMine(JT* pjt)
{
    MINE* pmine = pjt->pmine;

    if (pmine == nullptr)
        return;

    ALO* paloMineOrigin = nullptr;

    for (int i = 0; i < 4; ++i)
    {
        ALO* paloHat = pjt->apaloMineHat[i];

        if (paloHat != nullptr && FIsLoInWorld(paloHat))
        {
            paloMineOrigin = paloHat;
            break;
        }
    }

    if (paloMineOrigin == nullptr)
        paloMineOrigin = (ALO*)pjt->plboneMineOrigin;

    pmine->pvtlo->pfnAddLo(pmine);
    pmine->pvtalo->pfnSetAloParent(pmine, nullptr);
    pmine->pvtalo->pfnMatchAloOtherObject(pmine, paloMineOrigin);

    pmine->fSearch = true;
    pmine->tSearch = g_clock.t;

    if (paloMineOrigin != (ALO*)pjt->plboneMineOrigin)
        paloMineOrigin->pvtlo->pfnRemoveLo(paloMineOrigin);
}

bool FIgnoreMineIntersection(MINE* pmine, SO* pso)
{
    if (reinterpret_cast<JT*>(pso) == g_pjt)
        return true;

    return FIgnoreSoIntersection(pmine, pso);
}

void PresetMineAccel(MINE* pmine, float dt)
{
    PresetSoAccel(pmine, dt);

    float dtSearch = g_clock.t - pmine->tSearch;
    float uSearch = s_clqMineSearchBlend.g0 + dtSearch * (s_clqMineSearchBlend.g1 + dtSearch * s_clqMineSearchBlend.g2);
    uSearch = glm::clamp(uSearch, 0.0f, 1.0f);

    glm::mat3 matUpright;
    TiltMatUpright(&pmine->xf.mat, nullptr, &matUpright);

    CLQ clqRotationSpring =
    {
        s_clqMineRotationSpring.g0 * uSearch,
        s_clqMineRotationSpring.g1 * uSearch,
        s_clqMineRotationSpring.g2 * uSearch
    };

    CLQ clqAngularDamping =
    {
        s_clqMineAngularDamping.g0 * uSearch,
        s_clqMineAngularDamping.g1 * uSearch,
        s_clqMineAngularDamping.g2 * uSearch
    };

    glm::vec3 wTarget(0.0f);

    if (pmine->fSearch)
        wTarget = pmine->xf.mat[2] * R_MineAngularTarget;
    else
        clqRotationSpring = s_clqMineRotationDisabled;

    AccelSoTowardMatSpring(pmine, &matUpright, &clqRotationSpring, &wTarget, &clqAngularDamping, dt);

    glm::vec3 posTarget(0.0f);

    if (pmine->fSearch)
        posTarget = pmine->xf.mat[0] * R_MinePositionTarget;

    CLQ clqPositionSpring =
    {
        s_clqMinePositionSpring.g0 * uSearch,
        s_clqMinePositionSpring.g1 * uSearch,
        s_clqMinePositionSpring.g2 * uSearch
    };

    AccelSoTowardPosSpring(pmine, nullptr, nullptr, &posTarget, &clqPositionSpring, dt);
}

void AdjustMineNewXp(MINE* pmine, XP* pxp, int ixpd)
{
    AdjustBombNewXp(pmine, pxp, ixpd);
    pmine->fSearch = false;
}

void HandleMineMessage(MINE* pmine, int msgid, void* pv)
{
    HandleBombMessage(pmine, (MSGID)msgid, pv);

    if (msgid != 10)
        return;

    const WATERMSG* pmsg = static_cast<const WATERMSG*>(pv);
    MINE* pmineMessage = static_cast<MINE*>(pmsg->palo);

    if (pmineMessage == pmine)
        pmine->fSearch = false;
}

void DeleteMine(MINE* pmine)
{
    delete pmine;
}
