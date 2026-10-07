#include "smack.h"
#include "gui_layout.h"

namespace
{
glm::vec3 GetSmackTarget(const SMACKI* psmacki, const CM* pcm)
{
    const glm::vec3& authoredTarget = psmacki->posTarget;

    // The life-token target (-0.05, 0.8) is authored relative to the screen
    // center and must retain the original PS2 screen-space interpretation.
    // Coin and clue targets sit near opposite screen edges and must follow the
    // same edge scaling as their HUD counters on widescreen displays.
    if (glm::abs(authoredTarget.x) < 0.5f)
        return authoredTarget;

    constexpr float kPs2Width = 640.0f;
    constexpr float kPs2Height = 492.80002f;
    const GuiScale guiScale = GetGuiScale();

    const float authoredX = (authoredTarget.x * 0.5f + 0.5f) * kPs2Width;
    const float authoredY = (0.5f - authoredTarget.y * 0.5f) * kPs2Height;

    const float targetX = authoredTarget.x < 0.0f
        ? authoredX * guiScale.x
        : static_cast<float>(g_gl.width) -
            (kPs2Width - authoredX) * guiScale.x;
    const float targetY = authoredTarget.y > 0.0f
        ? authoredY * guiScale.y
        : static_cast<float>(g_gl.height) -
            (kPs2Height - authoredY) * guiScale.y;

    const float targetNdcX =
        targetX * (2.0f / static_cast<float>(g_gl.width)) - 1.0f;
    const float targetNdcY =
        1.0f - targetY * (2.0f / static_cast<float>(g_gl.height));

    // ConvertCmScreenToWorld uses the camera's authored screen ranges. Undo
    // the active projection here so the rendered pickup reaches that pixel.
    const float projectedScaleX = pcm->xScreenRange * pcm->matProj[0][0];
    const float projectedScaleY = pcm->yScreenRange * pcm->matProj[1][1];

    return glm::vec3(
        projectedScaleX != 0.0f ? targetNdcX / projectedScaleX : targetNdcX,
        projectedScaleY != 0.0f ? targetNdcY / projectedScaleY : targetNdcY,
        authoredTarget.z);
}
}

void InitSmack(SMACK* psmack, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(psmack, ppos, scale, psoTouch);

    SMACKI* psmacki = psmack->psmacki;

    ConvertCmWorldToScreen(g_pcm, ppos, &psmack->posScreen);

    psmack->v.x = GRandInRange(psmacki->vInitMin.x, psmacki->vInitMax.x);
    psmack->v.y = GRandInRange(psmacki->vInitMin.y, psmacki->vInitMax.y);
    psmack->v.z = GRandInRange(psmacki->vInitMin.z, psmacki->vInitMax.z);

    psmack->dtLifetime = DT_SmackLifetime;
    psmack->clqAlpha = CLQ{1.0, 0.0, 0.0};
    psmack->clqScale = psmacki->clqScale;
}

void ProjectSmackTransform(SMACK* psmack, float dt)
{
    if (psmack == nullptr || psmack->psmacki == nullptr || g_pcm == nullptr)
        return;

    float remainingLifetime = (psmack->tCreated + psmack->dtLifetime) - g_clock.t;
    float dtSegment = glm::max(dt, remainingLifetime);

    glm::vec3 posScreen{};
    glm::vec3 vScreen{};
    glm::vec3 posTarget = GetSmackTarget(psmack->psmacki, g_pcm);

    EvaluateBezierPos(dtSegment, dt, 1.0f, &psmack->posScreen, &psmack->v, &posTarget, &psmack->psmacki->vTarget, &posScreen, &vScreen, nullptr);

    psmack->posScreen = posScreen;
    psmack->v = vScreen;

    ConvertCmScreenToWorld(g_pcm, &psmack->posScreen, &psmack->pos);
}

void RenderSmack(SMACK* psmack, CM* pcm)
{
    if (psmack == nullptr || pcm == nullptr || psmack->psmacki == nullptr || psmack->dtLifetime <= 0.0f)
        return;

    glm::vec3 pos{};
    ConvertCmScreenToWorld(pcm, &psmack->posScreen, &pos);

    glm::mat3 dmat(1.0f);
    CalculateDmat(&psmack->mat, &pcm->mat, &dmat);

    float rotationAngle = 0.0f;
    glm::vec3 rotationAxis(0.0f, 0.0f, 1.0f);

    DecomposeRotateMatrixRad(&dmat, &rotationAngle, &rotationAxis);

    const float lifetimeRatio = (g_clock.t - psmack->tCreated) / psmack->dtLifetime;

    glm::mat3 matCameraRotation(1.0f);
    glm::mat3 matSpin(1.0f);

    if (glm::dot(rotationAxis, rotationAxis) > 0.00000001f)
        matCameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), rotationAngle * lifetimeRatio, glm::normalize(rotationAxis)));

    glm::vec3 spinAxis = psmack->psmacki->normalSpin;

    if (glm::dot(spinAxis, spinAxis) > 0.00000001f)
        matSpin = glm::mat3(glm::rotate(glm::mat4(1.0f), lifetimeRatio * glm::two_pi<float>(), glm::normalize(spinAxis)));

    const glm::mat3 mat = matCameraRotation * matSpin * psmack->mat;

    FRenderRipPosMat(psmack, pcm, &pos, const_cast<glm::mat3*>(&mat));
}

void UpdateSmack(SMACK* psmack, float dt)
{
    if (psmack->dtLifetime < g_clock.t - psmack->tCreated) {
		PFNSMACK pfnsmack = psmack->psmacki->pfnsmack;
		void* pv = psmack->pv;

        RemoveRip(psmack);

		if (pfnsmack != nullptr)
			pfnsmack(pv);
    }
}

float DT_SmackLifetime = 0.5;
VTSMACK g_vtsmack;
