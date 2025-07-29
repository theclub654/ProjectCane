#pragma once
#include "so.h"

extern bool loadEmitMesh;

class PROXY : public ALO
{
	public:
		DL dlProxyRoot;
		DLE dleProxy;
};

PROXY*NewProxy();
void InitSwProxyDl(SW* psw);
void InitProxy(PROXY *pproxy);
int  GetProxySize();
void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis);
void CloneProxy(PROXY* pproxy, PROXY* pproxyBase);
// Apply transformation to proxy ALO
void ApplyAloProxy(ALO* palo, PROXY* pproxyApply);
// Apply transformtion to a proxy object
void ApplySoProxy(SO* pso, PROXY* pproxyApply);
void DeleteProxy(PROXY *pproxy);