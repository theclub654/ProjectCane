#include "proc.h"
#include "frame.h"

std::shared_ptr<CProc> PprocNew()
{
    return std::make_shared <CProc>();
}

void CProc::CloneTo(CProc* pprocClone, CFrame* pframeClone)
{
    if (pprocClone == nullptr)
        return;

    if (pframeClone != nullptr)
    {
        // CFrame inherits enable_shared_from_this and is created by PframeNew().
        pprocClone->m_pframeOwner = pframeClone->shared_from_this();
        pprocClone->m_pframe = pframeClone;
    }
    else
    {
        // Original behavior: retain the source procedure's environment.
        pprocClone->m_pframeOwner = m_pframeOwner;
        pprocClone->m_pframe = pprocClone->m_pframeOwner
            ? pprocClone->m_pframeOwner.get()
            : m_pframe;
    }

    pprocClone->m_ppairFormals = m_ppairFormals;
    pprocClone->m_crefReq = m_crefReq;
    pprocClone->m_fVarArg = m_fVarArg;
    pprocClone->m_ppairCodeExpr = m_ppairCodeExpr;
}