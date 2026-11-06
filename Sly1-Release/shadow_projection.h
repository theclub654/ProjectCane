#pragma once
#include <glm/glm.hpp>

namespace CaneShadow
{
inline glm::mat4 ClipFromUv(const glm::mat4& worldToUv)
{

    const bool positiveConstantQ = worldToUv[0][3] == 0.0f &&
        worldToUv[1][3] == 0.0f && worldToUv[2][3] == 0.0f &&
        worldToUv[3][3] > 0.0f;
    const float sign = positiveConstantQ ? 1.0f : -1.0f;
    glm::mat4 uvToClip(0.0f);
    uvToClip[0][0] = uvToClip[1][1] = 2.0f * sign;
    uvToClip[3] = glm::vec4(-sign, -sign, 0.0f, sign);
    return uvToClip * worldToUv;
}
}
