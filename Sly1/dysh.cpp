#include "dysh.h"

void* NewDysh()
{
	return new DYSH;
}

void InitDysh(DYSH* pdysh)
{
	InitAlo(pdysh);
}

void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro)
{

}

void DeleteDysh(LO* plo)
{
	delete(DYSH*)plo;
}
