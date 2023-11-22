#include "jp.h"

void* NewJp()
{
	return new JP;
}

void InitJp(JP* pjp)
{
	InitStep(pjp);
}

void RenderJpAll(JP* pjp, CM* pcm, RO* pro)
{

}

void DeleteJp(LO* plo)
{
	delete (JP*)plo;
}
