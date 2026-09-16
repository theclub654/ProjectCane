#pragma once
#include "alo.h"
#include "dec.h"

class PROXY : public ALO
{
	public:
	DL dlProxyRoot;
	DLE dleProxy;

	std::vector<std::shared_ptr <PXR>> apxrProxyRoot;
};

static int numProxy = 0;

PROXY*NewProxy();
void  InitProxy(PROXY *pproxy);
int   GetProxySize();
void  LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void  CloneProxy(PROXY* pproxy, PROXY* pproxyBase);
void  RebuildClonedProxyRoots(ALO* palo, ALO* paloBase);
void  PostProxyLoad(PROXY *pproxy);
void  DeleteProxy(PROXY *pproxy);
