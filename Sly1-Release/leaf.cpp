#include "leaf.h"
#include "stuck.h"

void PostLeafEmit(LEAF* pleaf, EMITB* pemitb)
{
    glm::vec3 lookDirection(
        GRandInRange(-1.0f, 1.0f),
        GRandInRange(-1.0f, 1.0f),
        GRandInRange(0.1f, 0.9f) + 0.1f
    );

    LoadLookAtMatrix(&lookDirection, &pleaf->mat);
}

void ProjectLeafTransform(LEAF* pleaf, float dt)
{
    const glm::vec3 axisX = pleaf->mat[0];
    const float speedAlongX = glm::dot(pleaf->v, axisX);

    const float angularVelocityRaw = s_clqSvToSw.g0 + speedAlongX * (s_clqSvToSw.g1 + speedAlongX * s_clqSvToSw.g2);
    const float angularVelocity = glm::clamp(angularVelocityRaw, s_lmSw.gMin, s_lmSw.gMax);

    glm::mat3 dmat;
    LoadRotateMatrixRad(angularVelocity * dt, &pleaf->mat[1], &dmat);

    pleaf->v = dmat * pleaf->v;
    pleaf->mat = dmat * pleaf->mat;

    const float rvDamping = pleaf->emitdv.rvDamping;

    // Project velocity into the leaf's local X direction and its YZ plane.
    glm::vec3 vX = pleaf->mat[0] * glm::dot(pleaf->v, pleaf->mat[0]);
    const glm::vec3 vYZ = pleaf->v - vX;

    const float svX = glm::length(vX);

    if (svX > 0.0001f)
        vX /= svX;
    else
        vX = g_normalX;

    const float svXDamped = s_svLeafTarget + (1.0f + s_rvDampingX * dt) * (svX - s_svLeafTarget);
    const float yzDamping = 1.0f + rvDamping * dt;

    pleaf->v = vX * svXDamped + vYZ * yzDamping;

    // The leaf applies its own directional damping above.
    pleaf->emitdv.rvDamping = 0.0f;
    ProjectRipTransform(pleaf, dt);
    pleaf->emitdv.rvDamping = rvDamping;
}

int FBounceLeaf(LEAF* pleaf, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal)
{
    if (pleaf == nullptr || psoOther == nullptr || psoOther->paloRoot == nullptr)
        return 0;

    BASIC* pRoot = static_cast<BASIC*>(psoOther->paloRoot);

    if (FIsBasicDerivedFrom(pRoot, CID_FLY))
        return 0;

    if (FIsBasicDerivedFrom(pRoot, CID_STEP))
        return 0;

    STUCK* pstuck = nullptr;

    CreateStuck(pleaf, pleaf->paloRender, psoOther, ppos, pnormal, &pstuck);

    if (pstuck != nullptr)
        ConvertAloMat(nullptr, psoOther, &pstuck->mat, &pstuck->mat);

    RemoveRip(pleaf);
    return 1;
}

CLQ s_clqSvToSw{ 0.0, -0.0099999998, 0.0 };
LM s_lmSw{-10, 10};
float s_svLeafTarget = 200;
float s_rvDampingX = -1.0;
VTLEAF g_vtleaf;