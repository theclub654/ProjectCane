#pragma once
#include "ref.h"

struct VARB
{
    SYMID symid;
    CRef ref;
};

class CFrame : public std::enable_shared_from_this<CFrame>
{
    public:
    int m_cpframeParent;
    CFrame* m_apframeParent[4];
	std::shared_ptr<CFrame> m_apframeParentOwner[4];
    int m_cvarb;
    VARB m_avarb[64];
    int m_fVisited;

    void RefAddBinding(SYMID symid, CRef* pref);
    CRef RefSetBinding(SYMID symid, CRef* pref);
    int FFindBinding(SYMID symid, int fRecursive, CRef* pref);
    CRef* PrefFindBinding(SYMID symid, int fRecursive);
    void SetSingleParent(CFrame* pframeParent);
    void AddParent(CFrame* pframeParent);
	void AddParent(const std::shared_ptr<CFrame>& pframeParent);
    void CloneTo(CFrame* pframeClone);
};

std::shared_ptr <CFrame> PframeNew();
