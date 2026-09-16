#include "bublet.h"

void InitBublet(BUBLET* pbublet, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitDroplet(pbublet, ppos, scale, psoTouch);

    pbublet->clqScale.g0 = 0.33f;
    pbublet->emitdv.rvDamping = -8.0f;
    pbublet->emitdv.dv = g_psw->dvGravity * -0.2f;
}

void ProjectBubletTransform(BUBLET* pbublet, float dt)
{
    if (pbublet == nullptr)
        return;

    ProjectRipTransform(pbublet, dt);

    const float random = GRandInRange(0.0f, 1.0f);
    const float speedSquared = glm::dot(pbublet->v, pbublet->v);

    if (speedSquared <= 0.00000001f || random >= 0.3f)
        return;

    const glm::vec3 normal = pbublet->v / glm::sqrt(speedSquared);

    glm::vec3 perpendicular0;

    if (glm::abs(normal.x) < 0.9f)
        perpendicular0 = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
    else
        perpendicular0 = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f)));

    const glm::vec3 perpendicular1 = glm::normalize(glm::cross(normal, perpendicular0));

    const float maximumForce = (1.0f - random * 3.3333333f) * 300.0f + 100.0f;
    const float force = GRandInRange(100.0f, maximumForce);
    const float angle = GRandInRange(0.0f, glm::two_pi<float>());
    const float sinAngle = glm::sin(angle);
    const float cosAngle = glm::cos(angle);

    const glm::vec3 brownianVelocity = perpendicular0 * (sinAngle * force) + perpendicular1 * (cosAngle * force);

    pbublet->pos += brownianVelocity * dt;
}

void TouchBublet(BUBLET* pbublet, int fTouching)
{
	TouchDroplet(pbublet, (uint32_t)(fTouching == 0));
}

VTBUBLET g_vtbublet;