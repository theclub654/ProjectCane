#include "akvb.h"

void LoadAkvbFromBrx(int* pckvb, std::vector<KVB>& akvb, CBinaryInputStream* pbis)
{
    const int ckvb = pbis->U16Read();

    *pckvb = ckvb;
    akvb.resize(ckvb);

    for (KVB& kvb : akvb)
    {
        kvb.t = pbis->S16Read() * (1.0f / 60.0f);
        kvb.vec = pbis->ReadVector();
        kvb.dvecIn = pbis->ReadVector();
        kvb.dvecOut = pbis->ReadVector();
    }
}

void EvaluateAkvb(int ckvb, KVB* akvb, float t, float svt, GRFEVAL grfeval, glm::vec3* pvec, glm::vec3* pdvec)
{
    float dt;
    float dtSeg;
    KVB* pkvb;

    FindKey(t, grfeval, sizeof(KVB), ckvb, reinterpret_cast<char*>(akvb), &dt, &dtSeg, reinterpret_cast<void**>(&pkvb));
    EvaluateBezierPos(dtSeg, dt, svt, &pkvb->vec, &pkvb->dvecOut, &pkvb[1].vec, &pkvb[1].dvecIn, pvec, pdvec, nullptr);
}

void GetAkvbTimes(int ckvb, KVB* akvb, int* pct, std::vector <float>& pat)
{
    *pct = ckvb;

    pat.clear();
    pat.reserve(ckvb);

    for (int i = 0; i < ckvb; i++)
        pat.push_back(akvb[i].t);
}
