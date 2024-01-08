#pragma once
#include "alo.h"
#include "dec.h"

extern bool loadEmitMesh;

class PROXY : public ALO
{
	public:
		DL dlProxyRoot;
		DLE dleProxy;
};

static int ProxyCount = 0;

void*NewProxy();
void InitProxy(PROXY *pproxy);
int  GetProxySize();
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void CloneProxy(PROXY* pproxy, PROXY* pproxyBase);
void DeleteProxy(LO* plo);