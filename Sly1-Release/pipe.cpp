#include "pipe.h"

PIPE* NewPipe()
{
	return new PIPE{};
}

void InitPipe(PIPE* ppipe)
{
	InitShape(ppipe);
}

int GetPipeSize()
{
	return sizeof(PIPE);
}

void OnPipeAdd(PIPE* ppipe)
{
	OnLoAdd(ppipe);
}

void OnPipeRemove(PIPE* ppipe)
{
	OnLoRemove(ppipe);
	//RemoveDlEntry(&g_dlPipe, ppipe);
}

void ClonePipe(PIPE* ppipe, PIPE* ppipeBase)
{
	LO lo = *ppipe;
	*ppipe = *ppipeBase;
	memcpy(ppipe, &lo, sizeof(LO));

	CloneLo(ppipe, ppipeBase);
}

void DeletePipe(PIPE* ppipe)
{
	delete ppipe;
}
