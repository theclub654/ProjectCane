#include "ac.h"
#include "aseg.h"
#include "asega.h"
#include "crv.h"

void FindKey(float t, GRFEVAL grfeval, int cbKey, int ckey, char* abKey, float* pdt, float* pdtSeg, void** ppv)
{
    if (ppv == nullptr)
        return;

    *ppv = nullptr;

    if (abKey == nullptr || cbKey < static_cast<int>(sizeof(float)) || ckey < 2)
        return;

    int first = 1;
    int last = ckey;

    // Find the first key after t.
    while (first != last)
    {
        const int mid = (first + last) / 2;
        const float keyTime = *reinterpret_cast<const float*>(abKey + mid * cbKey);

        if (keyTime <= t)
            first = mid + 1;
        else
            last = mid;
    }

    int keyAfter = first;

    if (keyAfter >= ckey)
        keyAfter = ckey - 1;

    char* pKey = abKey + (keyAfter - 1) * cbKey;

    if ((grfeval & 1U) != 0 &&
        *reinterpret_cast<const float*>(pKey) == t)
    {
        pKey -= cbKey;

        if (pKey < abKey)
        {
            pKey += (ckey - 1) * cbKey;
            t = *reinterpret_cast<const float*>(pKey + cbKey);
        }
    }

    const float keyTime = *reinterpret_cast<const float*>(pKey);
    const float nextKeyTime =
        *reinterpret_cast<const float*>(pKey + cbKey);

    if (pdt != nullptr)
        *pdt = t - keyTime;

    if (pdtSeg != nullptr)
        *pdtSeg = nextKeyTime - keyTime;

    *ppv = pKey;
}

void EvaluateAposG(float g, int cpos, glm::vec3* apos, float* mpiposg, int fClosed, glm::vec3* ppos, glm::vec3* pnormTangent)
{
    float dg;
    float dgSeg;
    int ipos = IposFindAposG(g, cpos, mpiposg, fClosed, &dg, &dgSeg);

    if (ppos)
        *ppos = glm::mix(apos[ipos], apos[ipos + 1], dg / dgSeg);

    if (pnormTangent)
    {
        glm::vec3 tangent = apos[ipos + 1] - apos[ipos];
        float length = glm::length(tangent);

        *pnormTangent = length < 0.0001f ? g_normalX : tangent / length;
    }
}
