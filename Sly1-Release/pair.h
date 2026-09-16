#pragma once
#include <iostream>
#include "ref.h"

class CPair
{
    public:
    CRef m_ref;
    std::shared_ptr <CPair> m_ppairNext;

    void CloneTo(CPair* ppairClone, CFrame* pframeClone);
};

std::shared_ptr <CPair> PpairNew();