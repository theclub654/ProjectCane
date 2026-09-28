#include "circler.h"

std::shared_ptr<CIRCLER> NewCircler()
{
    return std::make_shared<CIRCLER>();
}

void LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis)
{
    pcircler->sw = pbis->F32Read();
    pcircler->sRadius = pbis->F32Read();
    pcircler->du = pbis->F32Read();
    pcircler->dv = pbis->F32Read();

    pbis->F32Read();
    pbis->F32Read();
}

void InitCircler(CIRCLER* pcircler, SAAF* psaaf)
{
    InitSaa(pcircler, psaaf);

    const uint32_t oldFlags = pcircler->sai.grfsai;
    pcircler->sai.grfsai = (oldFlags & ~1u) | 2u;
}

void UpdateCircler(CIRCLER* pcircler, float dt)
{
    if (!pcircler) return;

    if (pcircler->sai.pshd == nullptr)
        return;

    const float rad = RadNormalize(g_clock.t * pcircler->sw);

    float s = 0.0f;
    float c = 0.0f;
    CalculateSinCos(rad, &s, &c);

    const float duNew = s * pcircler->sRadius + pcircler->du;
    const float dvNew = c * pcircler->sRadius + pcircler->dv;

    SetSaiDuDv(&pcircler->sai, duNew, dvNew);
}

float UCompleteCircler(CIRCLER* pcircler)
{
    constexpr float TWO_PI = 6.283185f;
    constexpr float INV_TWO_PI = 0.1591549f;

    float angle = GModPositive(g_clock.t * pcircler->sw, TWO_PI);
    return angle * INV_TWO_PI;
}

void DeleteCircler(CIRCLER* pcircler)
{
    delete pcircler;
}
