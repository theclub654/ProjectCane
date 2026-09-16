#pragma once
#include <iostream>

class CProc
{
    public:
    CFrame* m_pframe = nullptr;
    std::shared_ptr <CFrame> m_pframeOwner;
    CPair* m_ppairFormals = nullptr;
    int m_crefReq = 0;
    int m_fVarArg = 0;
    CPair* m_ppairCodeExpr = nullptr;
    
    void CloneTo(CProc* pprocClone, CFrame* pframeClone);
};

std::shared_ptr <CProc> PprocNew();