#pragma once
#include "alo.h"

class DYSH : public ALO
{
	public:
};

void InitDysh(DYSH* pdysh);
void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro);