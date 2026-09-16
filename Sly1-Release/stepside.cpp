#include "stepside.h"
#include "jt.h"
#include "acp.h"

void PostJtLoadSidestep(JT* pjt, BLSS* ablss, ASEGBL** ppasegbl)
{
    constexpr int cbl = 8;
    const int cbBl = static_cast<int>(sizeof(BLSS));

    ablss[6].u = 1.0f;

    for (int i = 0; i < cbl; ++i)
    {
        ASEG* paseg = ablss[i].paseg;
        std::shared_ptr<ACP> pacpStrip = paseg->pchnStrip != nullptr ? paseg->pchnStrip->pacp : nullptr;

        if (pacpStrip == nullptr)
        {
            ALO* paloFoot = (i & 1) == 0 ? pjt->paloLeftFoot : pjt->paloRightFoot;
            CHN* pchnFoot = PchnFindAseg(paseg, paloFoot);
            std::shared_ptr<ACP> pacpFoot = pchnFoot->pacp;

            pacpFoot->pvtacp->pfnEvaluateAcp(pacpFoot.get(), paloFoot, 0.0f, 1.0f, static_cast<GRFEVAL>(0), nullptr, &ablss[i].v);
        }
        else
        {
            glm::vec3 posStart{};
            glm::vec3 posEnd{};

            pacpStrip->pvtacp->pfnEvaluateAcp(pacpStrip.get(), pjt, 0.0f, 1.0f, static_cast<GRFEVAL>(0), &posStart, nullptr);
            pacpStrip->pvtacp->pfnEvaluateAcp(pacpStrip.get(), pjt, paseg->tMax, 1.0f, static_cast<GRFEVAL>(0), &posEnd, nullptr);

            ablss[i].v = (posStart - posEnd) / paseg->tMax;

        }
    }

    EnsureAsegBlendDynamic(pjt, cbBl, cbl, ablss, 0, nullptr, nullptr, ppasegbl);
    ReblendAsegbl(*ppasegbl, cbBl, cbl, ablss);
}

void AdjustJtSidestep(JT* pjt, BLSS* ablss, ASEGBL* pasegbl)
{
    const float svtLocal = pjt->pasegaCur->svtLocal;
    if (pjt->sslfCur == SSLF_Nil)
        pjt->sslfCur = glm::dot(pjt->vTarget, ablss[0].v) < 0.0f ? SSLF_Left : SSLF_Right;

    for (int i = 0; i < 8; ++i)
        ablss[i].u = 0.0f;

    int iFirst = -1;
    int iIdle = -1;

    if (pjt->sslfCur == SSLF_Right)
    {
        iFirst = 0;
        iIdle = 6;
    }
    else if (pjt->sslfCur == SSLF_Left)
    {
        iFirst = 1;
        iIdle = 7;
    }

    if (iFirst >= 0)
    {
        const int indices[3] = { iFirst, iFirst + 2, iFirst + 4 };
        float weightSum = 0.0f;

        for (int index : indices)
        {
            const float velocityLengthSq = glm::dot(ablss[index].v, ablss[index].v);
            const float weight = glm::dot(pjt->vTarget, ablss[index].v) / (svtLocal * velocityLengthSq);

            ablss[index].u = std::max(weight, 0.0f);
            weightSum += ablss[index].u;
        }

        if (weightSum < 1.0f)
            ablss[iIdle].u = 1.0f - weightSum;
        else
        {
            ablss[iIdle].u = 0.0f;

            for (int index : indices)
                ablss[index].u /= weightSum;
        }
    }

    glm::vec3 blendedVelocity{ 0.0f };

    for (int i = 0; i < 8; ++i)
        blendedVelocity += ablss[i].v * ablss[i].u;

    pjt->vTarget = blendedVelocity * svtLocal;

    ReblendAsegbl(pasegbl, static_cast<int>(sizeof(BLSS)), 8, ablss);
}

void SetJtSidestepTarget(JT* pjt, BLSS* ablss, ASEGBL* pasegbl, float rad, float u)
{
    //(void)pasegbl;

    glm::vec3 vec{};
    SetVectorCylind(&vec, rad - pjt->radTarget, u, 0.0f);

    bool useLeft = false;

    if (vec.y > 10.0f)
    {
        if (pjt->sslfCur == SSLF_Right)
            vec.y = 0.0f;
        else
            useLeft = true;
    }
    else if (vec.y < -10.0f)
    {
        if (pjt->sslfCur == SSLF_Left)
        {
            vec.y = 0.0f;
            useLeft = true;
        }
    }
    else
        useLeft = pjt->sslfCur == SSLF_Left;

    const float forward = std::max(vec.x, 0.0f);
    const float backward = std::max(-vec.x, 0.0f);

    if (useLeft)
        pjt->vTarget = ablss[1].v * vec.y + ablss[3].v * forward + ablss[5].v * backward;
    else
        pjt->vTarget = ablss[0].v * -vec.y + ablss[2].v * forward + ablss[4].v * backward;

    pjt->vTarget.z = 0.0f;
    pjt->vTarget *= pjt->pasegaCur->svtLocal;
}
