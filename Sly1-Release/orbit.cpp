#include "orbit.h"

void InitOrbit(ORBIT* porbit, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(porbit, ppos, scale, psoTouch);

    porbit->dtLifetime = 2.0f;
    porbit->clqAlpha = CLQ{1.0, 0.0, 0.0};
}

void ProjectOrbitTransform(ORBIT* porbit, float dt)
{
    const float lifetimeRatio = glm::clamp((g_clock.t - porbit->tCreated) / porbit->dtLifetime, 0.0f, 1.0f);

    const float radiusTarget = 125.0f - lifetimeRatio * 25.0f;
    const float angularSpeedTarget = 20.0f + lifetimeRatio * 20.0f;

    const glm::vec3 posTarget(radiusTarget, 0.0f, 0.0f);
    const glm::vec3 vTarget(0.0f, angularSpeedTarget, 0.0f);

    const glm::vec3 dpos = posTarget - porbit->pos;
    const glm::vec3 dv = vTarget - porbit->v;
    const glm::vec3 acceleration = dpos * 25.0f + dv * 10.0f;

    porbit->v += acceleration * dt;

    glm::vec3 linearVelocity = porbit->v;
    linearVelocity.y = 0.0f;

    porbit->pos += linearVelocity * dt;
}

void UpdateOrbit(ORBIT* porbit, float dt)
{
    const glm::mat3 matOld = porbit->mat;
    const glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), porbit->v.y * dt, glm::vec3(0.0f, 0.0f, 1.0f)));
    const glm::mat3 matNew = rotation * matOld;

    bool crossedCameraPlane = false;

    if (g_pcm != nullptr)
    {
        const glm::vec3 cameraAxis = g_pcm->mat[0];
        const float dotOld = glm::dot(matOld[0], cameraAxis);
        const float dotNew = glm::dot(matNew[0], cameraAxis);

        crossedCameraPlane = dotOld > 0.0f && dotNew <= 0.0f && g_clock.t - porbit->tCreated > 0.5f;
    }

    if (crossedCameraPlane && porbit->palo != nullptr)
    {
        const glm::vec3 posWorld = porbit->palo->xf.posWorld + matNew * porbit->pos;
        RIP* psmack = PripNewRipg(RIPT_Smack, nullptr);

        if (psmack != nullptr)
        {
            if (psmack->pvtrip->pfnInitRip != nullptr)
                psmack->pvtrip->pfnInitRip(psmack, const_cast<glm::vec3*>(&posWorld), 1.0f, nullptr);

            psmack->dtLifetime = 0.25f;
        }

        --porbit->ctknv;

        if (porbit->ctknv < 1)
        {
            RemoveRip(porbit);
            return;
        }
    }

    porbit->mat = matNew;
}

VTORBIT g_vtorbit;