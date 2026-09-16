#pragma once
#include "ref.h"

struct SBB 
{
	int n;
	CRef ref;
};

class CSidebag
{
	public:
	int m_csbb;
	SBB m_asbb[16];

	CRef RefAddBinding(int n, CRef* pref);
	int  FFindBinding(int n, CRef* pref);
	CRef RefSetBinding(int n, CRef* pref);
	void CloneTo(CSidebag* psidebagClone);
};

std::shared_ptr <CSidebag> PsidebagNew();