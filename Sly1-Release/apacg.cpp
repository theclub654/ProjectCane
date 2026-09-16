#include "apacg.h"
#include "acgb.h"

void LoadApacgFromBrx(std::shared_ptr<ACG> apacg[3], glm::vec3& pvecDefault, CBinaryInputStream* pbis)
{
    uint8_t flags = pbis->U8Read();

    pvecDefault = pbis->ReadVector();

    for (int i = 0; i < 3; ++i)
    {
        bool hasAcg = (flags & 1) != 0;
        flags >>= 1;

        apacg[i].reset();

        if (!hasAcg)
            continue;

        ACGK acgk = static_cast<ACGK>(pbis->U8Read());

        apacg[i] = PacgNew(acgk);
        apacg[i]->pvtacg->pfnLoadAcgFromBrx(apacg[i].get(), pbis);
    }
}

void EvaluateApacg(std::shared_ptr<ACG> apacg[3], ALO* palo, float t, float svt, GRFEVAL grfeval, const glm::vec3* pvecDefault, glm::vec3* pvec, glm::vec3* pdvec)
{
    if (pvec != nullptr)
        *pvec = *pvecDefault;

    if (pdvec != nullptr)
        *pdvec = glm::vec3(0.0f);

    for (int i = 0; i < 3; ++i)
    {
        ACG* pacg = apacg[i].get();

        if (pacg == nullptr)
            continue;

        float* pg = pvec != nullptr ? &(*pvec)[i] : nullptr;
        float* pdg = pdvec != nullptr ? &(*pdvec)[i] : nullptr;

        pacg->pvtacg->pfnEvaluateAcg(pacg, palo, t, svt, grfeval, pg, pdg);
    }
}

void GetApacgTimes(std::shared_ptr<ACG> apacg[3], int* pct, std::vector<float>& pat)
{
    pat.clear();

    for (int i = 0; i < 3; ++i)
    {
        ACG* pacg = apacg[i].get();

        if (!pacg)
            continue;

        int ct = 0;
        std::vector<float> at;

        pacg->pvtacg->pfnGetAcgTimes(pacg, &ct, at);

        for (int j = 0; j < ct && j < static_cast<int>(at.size()); ++j)
            pat.push_back(at[j]);
    }

    std::stable_sort(pat.begin(), pat.end());

    pat.erase(std::unique(pat.begin(), pat.end()), pat.end());

    *pct = static_cast<int>(pat.size());
}
