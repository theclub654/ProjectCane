#include "frame.h"


std::shared_ptr <CFrame> PframeNew()
{
    return std::make_shared <CFrame>();
}

void CFrame::RefAddBinding(SYMID symid, CRef* pref)
{
    int index = m_cvarb++;

    VARB& varb = m_avarb[index];

    varb.symid = symid;
    varb.ref = *pref;

}

CRef CFrame::RefSetBinding(SYMID symid, CRef* pref)
{
    CRef ret;

    CRef* existing = PrefFindBinding(symid, 1);

    if (existing != nullptr && pref != nullptr)
        *existing = *pref;


    ret.SetTag(TAGK_Void);
    return ret;
}

int CFrame::FFindBinding(SYMID symid, int fRecursive, CRef* pref)
{
    CRef* found = PrefFindBinding(symid, fRecursive);

    if (found == nullptr)
    {
        return 0;
    }


    if (pref != nullptr)
        *pref = *found;

    return 1;
}

CRef* CFrame::PrefFindBinding(SYMID symid, int fRecursive)
{
    m_fVisited = 1;

    for (int i = 0; i < m_cvarb; i++)
    {
        if (m_avarb[i].symid == symid)
        {
            m_fVisited = 0;
            return &m_avarb[i].ref;
        }
    }

    if (fRecursive)
    {
        for (int i = 0; i < m_cpframeParent; i++)
        {
            CFrame* parent = m_apframeParent[i];

            if (parent == nullptr)
                continue;

            if (parent->m_fVisited)
                continue;

            CRef* found = parent->PrefFindBinding(symid, fRecursive);

            if (found != nullptr)
            {
                m_fVisited = 0;
                return found;
            }
        }
    }

    m_fVisited = 0;
    return nullptr;
}

void CFrame::SetSingleParent(CFrame* pframeParent)
{
    m_apframeParent[0] = pframeParent;
	m_apframeParentOwner[0] = pframeParent != nullptr
		? pframeParent->weak_from_this().lock()
		: std::shared_ptr<CFrame>{};
    m_cpframeParent = 1;
}

void CFrame::AddParent(CFrame* pframeParent)
{
	if (pframeParent == nullptr || m_cpframeParent >= 4)
		return;

    m_apframeParent[m_cpframeParent] = pframeParent;
	m_apframeParentOwner[m_cpframeParent] = pframeParent->weak_from_this().lock();
    ++m_cpframeParent;
}

void CFrame::AddParent(const std::shared_ptr<CFrame>& pframeParent)
{
	if (!pframeParent || m_cpframeParent >= 4)
		return;

	const int index = m_cpframeParent++;
	m_apframeParent[index] = pframeParent.get();
	m_apframeParentOwner[index] = pframeParent;
}

void CFrame::CloneTo(CFrame* pframeClone)
{
    pframeClone->m_cpframeParent = m_cpframeParent;

    for (int i = 0; i < m_cpframeParent && i < 4; ++i)
    {
        pframeClone->m_apframeParent[i] = m_apframeParent[i];
		pframeClone->m_apframeParentOwner[i] = m_apframeParentOwner[i];
    }

    pframeClone->m_cvarb = m_cvarb;
    pframeClone->m_fVisited = m_fVisited;

    for (int i = 0; i < m_cvarb && i < 64; ++i)
    {
        pframeClone->m_avarb[i].symid = m_avarb[i].symid;
        m_avarb[i].ref.CloneTo(&pframeClone->m_avarb[i].ref, pframeClone);
    }
}
