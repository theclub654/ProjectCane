#pragma once
#include "so.h"

class DART : public SO
{
	public:
};

void InitDart(DART* pdart);
void OnDartAdd(DART* pdart);
void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis);