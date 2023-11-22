#include "pipe.h"

void* NewPipe()
{
	return new PIPE;
}

void InitPipe(PIPE* ppipe)
{
	InitShape(ppipe);
}

void OnPipeAdd(PIPE* ppipe)
{
	OnLoAdd(ppipe);
}

void OnPipeRemove(PIPE* ppipe)
{
	OnLoRemove(ppipe);
}

void DeletePipe(LO* plo)
{
	delete(PIPE*)plo;
}
