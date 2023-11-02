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

static int LoadProxyFromBrxCount = 0;

void InitProxy(PROXY *pproxy);
void LoadProxyFromBrx(PROXY *pproxy, CBinaryInputStream *pbis);