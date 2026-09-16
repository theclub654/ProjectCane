#include "pair.h"

std::shared_ptr <CPair> PpairNew()
{
    return std::make_shared <CPair>();
}

void CPair::CloneTo(CPair* ppairClone, CFrame* pframeClone)
{
    if (!ppairClone)
        return;

    m_ref.CloneTo(&ppairClone->m_ref, pframeClone);

    if (m_ppairNext)
    {
        ppairClone->m_ppairNext = PpairNew();
        m_ppairNext->CloneTo(ppairClone->m_ppairNext.get(), pframeClone);
    }
    else
    {
        ppairClone->m_ppairNext = nullptr;
    }
}