#include "dysh.h"

void InitDysh(DYSH* pdysh)
{
	//std::cout << "DYSH Size: " << sizeof(DYSH) << "\n";
	InitAlo(pdysh);
}

void RenderDyshSelf(DYSH* pdysh, CM* pcm, RO* pro)
{

}

void DeleteDysh(LO* plo)
{
	delete(DYSH*)plo;
}
