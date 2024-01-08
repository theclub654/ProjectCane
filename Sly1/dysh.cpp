#include "dysh.h"

void* NewDysh()
{
	return new DYSH;
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
	LO lo = *pdysh;
	*pdysh = *pdyshBase;
	memcpy(pdysh, &lo, sizeof(LO));

	CloneLo(pdysh, pdyshBase);

	ClearDl(&pdysh->dlChild);
}

void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro)
{

}

void DeleteDysh(LO* plo)
{
	delete(DYSH*)plo;
}
