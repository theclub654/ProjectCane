#pragma once
#include "alo.h"

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
void DeleteProxy(PROXY *pproxy);