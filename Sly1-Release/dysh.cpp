#include "dysh.h"

DYSH* NewDysh()
{
	return new DYSH{};
}

void InitDysh(DYSH* pdysh)
{
	InitAlo(pdysh);
}

int GetDyshSize()
{
	return sizeof(DYSH);
}

void CloneDysh(DYSH* pdysh, DYSH* pdyshBase)
{
	CloneAlo(pdysh, pdyshBase);

	pdysh->pshadowGen = pdyshBase->pshadowGen;
}

void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro)
{

}

void DeleteDysh(DYSH *pdysh)
{
	delete pdysh;
}
