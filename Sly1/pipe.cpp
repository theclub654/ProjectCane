#include "pipe.h"

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
