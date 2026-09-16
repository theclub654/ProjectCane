#include "bez.h"
#include "crv.h"

float SBezierPosLength(float dtSeg, float tSeg, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1)
{
    std::vector <glm::vec3> apos(20);

    TesselateBezier(dtSeg, 0.0f, tSeg, ppos0, pv0, ppos1, pv1, 20, apos.data());

    return SMeasureApos(20, apos.data(), nullptr);
}

void TesselateBezier(float dtSeg, float tStart, float tEnd, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, int cpos, glm::vec3* apos)
{
    float step = (tEnd - tStart) / float(cpos - 1);

    for (int i = 0; i < cpos; i++)
    {
        EvaluateBezierPos(dtSeg, tStart, 1.0f, ppos0, pv0, ppos1, pv1, &apos[i], nullptr, nullptr);
        tStart += step;
    }
}

void EvaluateBezierPos(float dtSeg, float tSeg, float svt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pdv)
{
    float t = tSeg / dtSeg;
    float u = 1.0f - t;

    if (ppos)
    {
        float t2 = t * t;
        float t3 = t2 * t;
        float u2 = u * u;
        float u3 = u2 * u;

        float a = u3 + 3.0f * t * u2;
        float b = t * u2 * dtSeg;
        float c = -(t2 * u) * dtSeg;
        float d = t3 + 3.0f * t2 * u;

        *ppos = (*ppos0) * a + (*pv0) * b + (*pv1) * c + (*ppos1) * d;
    }

    if (pv)
    {
        glm::vec3 dpos = (*ppos1) - (*ppos0);

        float a = (t * (6.0f - t * 6.0f)) / dtSeg;
        float b = 1.0f - t * (4.0f - t * 3.0f);
        float c = t * (t * 3.0f - 2.0f);

        *pv = (dpos * a + (*pv0) * b + (*pv1) * c) * svt;
    }

    if (pdv)
    {
        glm::vec3 dpos = (*ppos1) - (*ppos0);

        float a = (6.0f - t * 12.0f) / dtSeg;
        float b = t * 6.0f - 4.0f;
        float c = t * 6.0f - 2.0f;

        *pdv = (dpos * a + (*pv0) * b + (*pv1) * c) * ((svt * svt) / dtSeg);
    }
}

void EvaluateBezierWeightedFloat(float dtSeg, float tSeg, float svt, float g0, float dt0, float gCP0, float g1, float dt1, float gCP1, float* pg, float* pdg, float* pddg)
{
    float u = 0.0f;

    if (tSeg < 0.0001f)
        u = 0.0f;
    else if (dtSeg - tSeg < 0.0001f)
        u = 1.0f;
    else
    {
        const float tCP0 = dt0 * 3.0f;
        const float tCP1 = (dtSeg - dt1) * 3.0f;
        float uMin = 0.0f;
        float uMax = 1.0f;

        for (int i = 1; i <= 16; ++i)
        {
            const float testU = uMin + 1.0f / static_cast<float>(1U << i);
            const float bezT = testU * (testU * (testU * ((tCP0 - tCP1) + dtSeg) + ((tCP1 - tCP0) - tCP0)) + tCP0) - tSeg;

            if (bezT < 0.0f)
                uMin = testU;
            else if (bezT > 0.0f)
                uMax = testU;
            else
            {
                uMin = testU;
                uMax = testU;
                break;
            }
        }

        u = (uMin + uMax) * 0.5f;
    }

    if (pg != nullptr)
        *pg = u * (u * (u * (((gCP0 * 3.0f - g0) - gCP1 * 3.0f) + g1) + (g0 * 3.0f - gCP0 * 6.0f) + gCP1 * 3.0f) + g0 * -3.0f + gCP0 * 3.0f) + g0;

    if (pdg != nullptr)
    {
        const float tCP0 = dt0 * 3.0f;
        const float tCP1 = (dtSeg - dt1) * 3.0f;
        const float dgdu = u * (u * (((g0 * -3.0f + gCP0 * 9.0f) - gCP1 * 9.0f) + g1 * 3.0f) + (g0 * 6.0f - gCP0 * 12.0f) + gCP1 * 6.0f) + g0 * -3.0f + gCP0 * 3.0f;
        const float dtdu = u * (u * ((tCP0 * 3.0f - tCP1 * 3.0f) + dtSeg * 3.0f) + tCP0 * -4.0f + tCP1 * 2.0f) + tCP0;

        *pdg = std::abs(dtdu) > 0.000001f ? dgdu * svt / dtdu : 0.0f;
    }

    if (pddg != nullptr)
        *pddg = 0.0f;
}

void EvaluateBezierFloat(float dtSeg, float tSeg, float svt, float g0, float gSlope0, float g1, float gSlope1, float* pg, float* pdg, float* pddg)
{
    float u = tSeg / dtSeg;
    float v = 1.0f - u;

    if (pg)
    {
        float uvv = u * v * v;
        float uuv = u * u * v;
        *pg = ((v * v * v + uvv * 3.0f) * g0 + uvv * dtSeg * gSlope0) - (uuv * dtSeg * gSlope1) + ((u * u * u + uuv * 3.0f) * g1);
    }

    if (pdg)
        *pdg = (((u * v * 6.0f) / dtSeg) * (g1 - g0) + (1.0f - u * (4.0f - u * 3.0f)) * gSlope0 + u * (u * 3.0f - 2.0f) * gSlope1) * svt;

    if (pddg)
        *pddg = (((6.0f - u * 12.0f) / (dtSeg * dtSeg)) * (g1 - g0) + ((u * 6.0f - 4.0f) / dtSeg) * gSlope0 + ((u * 6.0f - 2.0f) / dtSeg) * gSlope1) * svt * svt;
}

void EvaluateBezierMat(float dtSeg, float tSeg, float svt, const glm::mat3& mat0, const glm::vec3& w0, const glm::mat3& mat1, const glm::vec3& w1, glm::mat3* pmat, glm::vec3* pw, glm::vec3* pdw)
{
    constexpr float kTiny = 0.0001f;
    const glm::vec3 normalZ(0.0f, 0.0f, 1.0f);

    if (dtSeg <= kTiny)
    {
        if (pmat != nullptr)
            *pmat = mat0;

        if (pw != nullptr)
            *pw = w0 * svt;

        if (pdw != nullptr)
            *pdw = glm::vec3(0.0f);

        return;
    }

    const float t = glm::clamp(tSeg / dtSeg, 0.0f, 1.0f);
    const float t2 = t * t;
    const float u = 1.0f - t;
    const float u2 = u * u;

    if (pmat != nullptr)
    {
        const float speed0 = glm::length(w0);
        const glm::vec3 axis0 = speed0 < kTiny ? normalZ : w0 / speed0;
        const glm::mat3 rot0 = glm::mat3_cast(glm::angleAxis(t * u2 * dtSeg * speed0, axis0));
        const glm::mat3 bezMat0 = rot0 * mat0;

        const float speed1 = glm::length(w1);
        const glm::vec3 axis1 = speed1 < kTiny ? normalZ : w1 / speed1;
        const glm::mat3 rot1 = glm::mat3_cast(glm::angleAxis(-t2 * u * dtSeg * speed1, axis1));
        const glm::mat3 bezMat1 = rot1 * mat1;

        const glm::mat3 dmat = bezMat1 * glm::transpose(bezMat0);
        glm::quat q = glm::normalize(glm::quat_cast(dmat));

        if (q.w < 0.0f)
            q = -q;

        float rad = glm::angle(q);
        glm::vec3 axis = glm::axis(q);

        if (!std::isfinite(rad) || glm::length2(axis) < kTiny * kTiny)
        {
            rad = 0.0f;
            axis = normalZ;
        }

        const float blendAmount = t2 * (3.0f - 2.0f * t);
        const glm::mat3 blend = glm::mat3_cast(glm::angleAxis(rad * blendAmount, axis));

        *pmat = blend * bezMat0;
    }

    if (pw != nullptr || pdw != nullptr)
    {
        const glm::mat3 dmat = mat1 * glm::transpose(mat0);
        glm::quat q = glm::normalize(glm::quat_cast(dmat));

        if (q.w < 0.0f)
            q = -q;

        float rad = glm::angle(q);
        glm::vec3 axis = glm::axis(q);

        if (!std::isfinite(rad) || glm::length2(axis) < kTiny * kTiny)
        {
            rad = 0.0f;
            axis = normalZ;
        }

        if (pw != nullptr)
        {
            const float cDelta = rad * t * (6.0f - 6.0f * t) / dtSeg;
            const float cW0 = 1.0f - t * (4.0f - 3.0f * t);
            const float cW1 = t * (3.0f * t - 2.0f);

            *pw = (axis * cDelta + w0 * cW0 + w1 * cW1) * svt;
        }

        if (pdw != nullptr)
        {
            const float cDelta = rad * (6.0f - 12.0f * t) / (dtSeg * dtSeg);
            const float cW0 = (6.0f * t - 4.0f) / dtSeg;
            const float cW1 = (6.0f * t - 2.0f) / dtSeg;

            *pdw = (axis * cDelta + w0 * cW0 + w1 * cW1) * (svt * svt);
        }
    }
}
void LimitBezierMulti(int c, float* xValues, double xAccelerationMax, float* yValues, double yAccelerationMax, float* zValues, double zAccelerationMax, float* pdtMin)
{
    struct INTERVAL
    {
        float tMin;
        float tMax;
    };

    const std::array<float*, 3> componentValues =
    {
        xValues,
        yValues,
        zValues
    };

    const std::array<float, 3> accelerationLimits =
    {
        static_cast<float>(xAccelerationMax),
        static_cast<float>(yAccelerationMax),
        static_cast<float>(zAccelerationMax)
    };

    const int componentCount = std::clamp(c, 0, 3);

    std::vector<INTERVAL> intervals;
    intervals.reserve(componentCount * 4);

    auto addInterval = [&intervals](float a, float b, float constant)
        {
            float roots[2];

            if (CSolveQuadratic(a, b, constant, roots) != 2)
                return;

            if (roots[1] < roots[0])
                std::swap(roots[0], roots[1]);

            intervals.push_back({ roots[0], roots[1] });
        };

    for (int i = 0; i < componentCount; ++i)
    {
        const float* values = componentValues[i];
        const float accelerationMax = accelerationLimits[i];

        const float posStart = values[0];
        const float velocityStart = values[1];
        const float posEnd = values[2];
        const float velocityEnd = values[3];

        const float startVelocityCoefficient = velocityStart * 4.0f + velocityEnd * 2.0f;
        const float startPositionCoefficient = posStart * 6.0f - posEnd * 6.0f;

        const float endVelocityCoefficient = velocityStart * -2.0f - velocityEnd * 4.0f;
        const float endPositionCoefficient = posStart * -6.0f + posEnd * 6.0f;

        addInterval(accelerationMax, startVelocityCoefficient, startPositionCoefficient);
        addInterval(-accelerationMax, startVelocityCoefficient, startPositionCoefficient);
        addInterval(accelerationMax, endVelocityCoefficient, endPositionCoefficient);
        addInterval(-accelerationMax, endVelocityCoefficient, endPositionCoefficient);
    }

    constexpr float epsilon = 0.0001f;
    float dtMin = FLT_MAX;

    for (int candidateIndex = -1; candidateIndex < static_cast<int>(intervals.size()); ++candidateIndex)
    {
        const float candidate = candidateIndex < 0 ? 0.0f : intervals[candidateIndex].tMax;

        if (dtMin < candidate)
            continue;

        bool candidateValid = true;

        for (const INTERVAL& interval : intervals)
        {
            if (interval.tMin + epsilon < candidate && candidate < interval.tMax - epsilon)
            {
                candidateValid = false;
                break;
            }
        }

        if (candidateValid)
            dtMin = candidate;
    }

    *pdtMin = dtMin;
}
