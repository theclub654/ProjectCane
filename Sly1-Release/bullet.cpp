#include "bullet.h"
#include "jt.h"

void PostBulletEmit(BULLET* pbullet, EMITB* pemitb)
{
    ConvertAloPos(pemitb->emito.paloReference, nullptr, &pemitb->emito.posOrigin, &pbullet->pos);

    pbullet->emitdv.rvDamping = 0.0f;
    pbullet->rwDamping = 0.0f;
    pbullet->fRichochet = pemitb->emitx.bullet.fRichochet;
    pbullet->fDamage = pemitb->emitx.bullet.fDamage;
}

void RenderBullet(BULLET* pbullet, CM* pcm)
{
    if (pbullet->paloRender == nullptr)
    {
        const float speedSquared = glm::dot(pbullet->v, pbullet->v);

        if (speedSquared <= 0.0001f)
            return;

        glm::vec3 direction = pbullet->v / std::sqrt(speedSquared);
        glm::vec3 posNext = pbullet->pos + direction * 100.0f;

        RenderAloLine(static_cast<ALO*>(g_psw->aploStock[11]), pcm, &posNext, &pbullet->pos, 10.0f, 1.0f);
        return;
    }

    glm::mat3 matRender;
    BuildOrthonormalMatrixY(&pbullet->v, &pcm->mat[0], &matRender);
    FRenderRipPosMat(pbullet, pcm, &pbullet->pos, &matRender);
}

int FBounceBullet(BULLET* pbullet, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal)
{
    if (pbullet == nullptr)
        return 1;

    if (pbullet->fDamage)
    {
        WKR wkr{};

        wkr.v = pbullet->v;
        wkr.pos = *ppos;
        wkr.ploSource = static_cast<LO*>(pbullet->pripg);
        wkr.ploTarget = static_cast<LO*>(psoOther);
        wkr.grftak = 16;

        if (PaloAbsorbWkr(&wkr, 0, nullptr) != nullptr)
        {
            RemoveRip(pbullet);
            return 1;
        }
    }

    if (!pbullet->fRichochet)
    {
        RemoveRip(pbullet);
        return 1;
    }

    pbullet->pos = *ppos;

    glm::mat3 matBounce;
    LoadLookAtMatrix(pnormal, &matBounce);

    // Convert into collision-normal space.
    glm::vec3 vLocal = glm::transpose(matBounce) * pbullet->v;

    // Reverse and damp the velocity component facing the surface.
    vLocal.x *= GRandInRange(-0.95f, -0.85f);

    // Add randomized tangential deflection.
    vLocal.y += GRandInRange(-400.0f, 400.0f);
    vLocal.z += GRandInRange(-400.0f, 400.0f);

    // Convert back into world space.
    pbullet->v = matBounce * vLocal;

    return 1;
}

VTBULLET g_vtbullet;